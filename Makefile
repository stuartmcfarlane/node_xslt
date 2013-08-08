REPORTER = dot

test:
	@NODE_ENV=test ./node_modules/.bin/jasmine-node --matchall --verbose test

test-w:
	@NODE_ENV=test ./node_modules/.bin/jasmine-node --matchall --autotest test

.PHONY: test test-w