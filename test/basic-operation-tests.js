var jasmine = require('jasmine-node');

describe('node_xslt', function(){

    it('exists', function() {
        // Just require node_xslt and see if it is defined
        xslt = require('../');
        expect(xslt).toBeDefined();
    });
});
