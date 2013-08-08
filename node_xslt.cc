#include <node.h>
#include <v8.h>
#include <libxml/HTMLparser.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libexslt/exslt.h>
#include <string.h>
#include "macros.h"
#include "scopeguard.h"
#include <iostream>

#define OBJ_DESTRUCTOR(d) Persistent<Object> _weak_handle = Persistent<Object>::New(self); \
                          _weak_handle.MakeWeak(NULL, d);

using namespace v8;
using namespace node;

void jsXmlDocCleanup(Persistent<Value> value, void *) {
    HandleScope handlescope;
    Local<Object> obj = value->ToObject();
    EXTERNAL(xmlDocPtr, doc, obj, 0);
    xmlFreeDoc(doc);
    return;
}

void jsXsltStylesheetCleanup(Persistent<Value> value, void *) {
    HandleScope handlescope;
    Local<Object> obj = value->ToObject();
    EXTERNAL(xsltStylesheetPtr, style, obj, 0);
    xsltFreeStylesheet(style);
    return;
}

OBJECT(jsXmlDoc, 1, xmlDocPtr doc)
    INTERNAL(0, doc)
    OBJ_DESTRUCTOR(&jsXmlDocCleanup)
    RETURN_SCOPED(self);
END

OBJECT(jsXsltStylesheet, 1, xsltStylesheetPtr style)
    INTERNAL(0, style)
    OBJ_DESTRUCTOR(&jsXsltStylesheetCleanup)
    RETURN_SCOPED(self);
END

// ------------------------------------------------------------------------------------------------------------------------

class Worker {
public:
    int result;
    v8::Persistent<v8::Function> callback;
    std::string error;
    virtual void MapArguments(v8::Arguments const &args) = 0;
    virtual v8::Handle<v8::Value> GetWorkerResult() = 0;
    virtual int Execute() = 0;

    void SetCallback(v8::Arguments const &args) {
        int cbIdx = args.Length() - 1;
        if (cbIdx > 0 && args[cbIdx]->IsFunction()) {
            Local<Function> _callback = Local<Function>::Cast(args[cbIdx]);
            callback = Persistent<Function>::New(_callback);
        }
    }
    static void WorkerFn(uv_work_t* req) {
        Worker* worker = (Worker*)req->data;
        worker->result = worker->Execute();
    }

    static void WorkerAfterFn(uv_work_t* req, int)
    {
        HandleScope scope;
        Worker* worker = (Worker*)req->data;
        delete req;
        Handle<Value> argv[2];

        if (0 != worker->result) {
            argv[0] = v8::String::New(worker->error.c_str());
            argv[1] = Undefined();
        }
        else {
            argv[0] = Undefined();
            argv[1] = worker->GetWorkerResult();
        }

        TryCatch try_catch;
        worker->callback->Call(Context::GetCurrent()->Global(), 2, argv);
        if (try_catch.HasCaught()) 
        {
            FatalException(try_catch);
        }
        worker->callback.Dispose();

        delete worker;
    }

};

// ------------------------------------------------------------------------------------------------------------------------

class ReadXmlFileWorker : public Worker {
public:
    std::string filename;
    xmlDocPtr doc;

    virtual void MapArguments(v8::Arguments const &args) {
        ARG_COUNT_ASYNC(1);
        ARG_utf8(str, 0);
        SetCallback(args);

        filename = *str;

    }
    virtual v8::Handle<v8::Value> GetWorkerResult() {
        v8::Handle<v8::Value> workerResult = jsXmlDoc(doc);
        return workerResult;
    }
    virtual int Execute() {
        doc = xmlReadFile(filename.c_str(), "UTF-8", 0);
        if (!doc) {
            error = "Failed to parse XML";
            return -1;
        }
        return 0;
    }
};

// ------------------------------------------------------------------------------------------------------------------------

FUNCTION(readXmlString)
    ARG_COUNT(1)
    ARG_utf8(str, 0)

    xmlDocPtr doc = xmlReadMemory(*str, str.length(), NULL, "UTF-8", 0);
    if (!doc) {
        return JS_ERROR("Failed to parse XML");
    }
    RETURN_SCOPED(jsXmlDoc(doc));
END

// ------------------------------------------------------------------------------------------------------------------------

class ReadHtmlFileWorker : public Worker {
public:
    std::string filename;
    htmlDocPtr doc;

    virtual void MapArguments(v8::Arguments const &args) {
        ARG_COUNT_ASYNC(1);
        ARG_utf8(str, 0);
        SetCallback(args);
        filename = *str;
    }
    virtual v8::Handle<v8::Value> GetWorkerResult() {
        v8::Handle<v8::Value> workerResult = jsXmlDoc(doc);
        return workerResult;
    }
    virtual int Execute() {
        doc = htmlReadFile(filename.c_str(), "UTF-8", 0);
        if (!doc) {
            error = "Failed to parse HTML";
            return -1;
        }
        return 0;
    }
};

// ------------------------------------------------------------------------------------------------------------------------

FUNCTION(readHtmlString)
    ARG_COUNT(1)
    ARG_utf8(str, 0)

    htmlDocPtr doc = htmlReadMemory(*str, str.length(), NULL, "UTF-8", HTML_PARSE_RECOVER);
    if (!doc) {
        return JS_ERROR("Failed to parse HTML");
    }
    RETURN_SCOPED(jsXmlDoc(doc));
END

// ------------------------------------------------------------------------------------------------------------------------

class ReadXsltFileWorker : public Worker {
public:
    std::string filename;
    xsltStylesheetPtr stylesheet;

    virtual void MapArguments(v8::Arguments const &args) {
        ARG_COUNT_ASYNC(1);
        ARG_utf8(str, 0);
        SetCallback(args);
        filename = *str;
    }
    virtual v8::Handle<v8::Value> GetWorkerResult() {
        v8::Handle<v8::Value> workerResult = jsXsltStylesheet(stylesheet);
        return workerResult;
    }
    virtual int Execute() {
        xmlDocPtr doc = xmlReadFile(filename.c_str(), "UTF-8", 0);
        if (!doc) {
            error = "Failed to parse XML";
            return -1;
        }
        stylesheet = xsltParseStylesheetDoc(doc);
        if (!stylesheet) {
            error = "Failed to parse stylesheet";
            return -2;
        }
        return 0;
    }
};

// ------------------------------------------------------------------------------------------------------------------------

FUNCTION(readXsltString)
    ARG_COUNT(1)
    ARG_utf8(str, 0)

    xmlDocPtr doc = xmlReadMemory(*str, str.length(), NULL, "UTF-8", 0);
    if (!doc) {
        return JS_ERROR("Failed to parse XML");
    }
    ScopeGuard guard = MakeGuard(xmlFreeDoc, doc);

    xsltStylesheetPtr stylesheet = xsltParseStylesheetDoc(doc);
    if (!stylesheet) {
        return JS_ERROR("Failed to parse stylesheet");
    }
    guard.Dismiss();
    RETURN_SCOPED(jsXsltStylesheet(stylesheet));
END

// ------------------------------------------------------------------------------------------------------------------------

class TransformWorker : public Worker {
public:
    xmlDocPtr document;
    xsltStylesheetPtr stylesheet;
    char **params;
    int paramsSize;
    std::string transformResult;

    virtual ~TransformWorker() {
        FreeArray();
    }
    void SetStylesheet(v8::Local<v8::Value> arg) {
        v8::Handle<v8::Object> objStylesheet= arg->ToObject();
        stylesheet =  (xsltStylesheetPtr) (v8::Local<v8::External>::Cast(objStylesheet->GetInternalField(0))->Value());
    }
    void SetDocument(v8::Local<v8::Value> arg) {
        v8::Handle<v8::Object> objDocument = arg->ToObject();
        document =  (xmlDocPtr) (v8::Local<v8::External>::Cast(objDocument->GetInternalField(0))->Value());
    }
    void SetArray(v8::Local<v8::Value> arg) {
        if (!arg->IsArray()) {
            throw std::string("Third parameter must be an array");
        }
        v8::Handle<v8::Array> array = v8::Handle<v8::Array>::Cast(arg);
        paramsSize = array->Length();

        if (paramsSize % 2 != 0) {
            throw "Array contains an odd number of parameters";
        }

        params = (char **)malloc(sizeof(char *) * (paramsSize + 1));
        if (!params) {
            throw "Failed to allocate memory";
        }
        memset(params, 0, sizeof(char *) * (paramsSize + 1));

        for (int i = 0; i < paramsSize; i++) {
            Local<String> param = array->Get(JS_int(i))->ToString();
            params[i] = (char *)malloc(sizeof(char) * (param->Length() + 1));
            if (!params[i]) {
                throw "Failed to allocate memory";
            }
            param->WriteAscii(params[i]);
        }
    }
    void FreeArray() {
        for (int i = 0; i < paramsSize; i++) {
            if (params[i]) {
                free(params[i]);
            }
            params[i] = 0;
        }
        free(params);
        params = 0;
    }
    virtual void MapArguments(v8::Arguments const &args) {
        ARG_COUNT_ASYNC(3)
        SetStylesheet(args[0]);
        SetDocument(args[1]);
        SetArray(args[2]);
        SetCallback(args);
    }
    virtual v8::Handle<v8::Value> GetWorkerResult() {
        return v8::Handle<v8::String>(v8::String::New(transformResult.c_str()));
    }
    virtual int Execute() {
        try {
            xmlDocPtr doc = xsltApplyStylesheet(stylesheet, document, (const char **)params);
            if (!doc) {
                error = "Failed to apply stylesheet";
                return -1;
            }
            ON_BLOCK_EXIT(xmlFreeDoc, doc);

            xmlChar *doc_ptr;
            int doc_len;
            xsltSaveResultToString(&doc_ptr, &doc_len, doc, stylesheet);

            if (doc_ptr) {
                ON_BLOCK_EXIT(xmlFree, doc_ptr);
                transformResult = (char*)doc_ptr;
                return 0;
            } else {
                return 0;
            }
        } catch (Handle<Value> err) {
            error = "Oops." /*err*/;
            return -1;
        }
        return 0;
    }
};

// ------------------------------------------------------------------------------------------------------------------------

template<class W>
static v8::Handle<v8::Value> SyncFunction(const v8::Arguments& args) {
    v8::HandleScope handlescope;
    Worker *worker = new W();
    try {
        worker->MapArguments(args);
        int result = worker->Execute();
        if (0 != result) {
            std::string error = worker->error;
            delete worker;
            return JS_ERROR(error.c_str());
        }
        v8::Handle<v8::Value> workerResult = worker->GetWorkerResult();
        delete worker;
        RETURN_SCOPED(workerResult);
    }
    catch (std::string error) {
        delete worker;
        return ThrowException(v8::String::New(error.c_str()));
    }
}

// ------------------------------------------------------------------------------------------------------------------------

template<class W>
static v8::Handle<v8::Value> AsyncFunction(const v8::Arguments& args) {
    v8::HandleScope handlescope;
    HandleScope scope;
    Worker* worker = new W();
    worker->MapArguments(args);
    uv_work_t* req = new uv_work_t();
    req->data = worker;
    uv_queue_work(uv_default_loop(), req, Worker::WorkerFn, Worker::WorkerAfterFn);
    return Undefined();
}

// ------------------------------------------------------------------------------------------------------------------------

template<class W>
static v8::Handle<v8::Value> SyncAsyncFunction(const v8::Arguments& args) {
    if (args.Length() > 0 && args[args.Length() - 1]->IsFunction()) {
        return AsyncFunction<W>(args);
    }
    return SyncFunction<W>(args);
}

// ------------------------------------------------------------------------------------------------------------------------

extern "C" void init(Handle<Object> target)
{
    HandleScope scope;

    exsltRegisterAll();
 
    Handle<Object> self = target;
    BIND("readXmlString", readXmlString);
    BIND("readXmlFile", SyncAsyncFunction<ReadXmlFileWorker> );
    BIND("readHtmlString", readHtmlString);
    BIND("readHtmlFile", SyncAsyncFunction<ReadHtmlFileWorker>);
    BIND("readXsltString", readXsltString);
    BIND("readXsltFile", SyncAsyncFunction<ReadXsltFileWorker>);
    BIND("transform", SyncAsyncFunction<TransformWorker>);
}

NODE_MODULE(node_xslt, init)