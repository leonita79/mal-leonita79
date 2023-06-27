const read_str = require('./reader.js');
const pr_str = require('./printer.js');
const Env = require('./env.js');
const fs = require('fs');
const { Console } = require('console');
const { arrayBuffer } = require('stream/consumers');

function mal_equals(a, b) {
    if (a === b
        || (a == null && b == null)
    ) {
        return true;
    } else if (Array.isArray(a) && Array.isArray(b)
        && a.length === b.length
    ) {
        return a.slice(1).every((value, index) => mal_equals(value, b[index+1]));
    } else if (typeof a === 'symbol' && typeof b === 'symbol') {
        return a.description === b.description;
    } else if (typeof a === 'object' && typeof b === 'object'
        && a != null && b != null
        && Object.keys(a).length === Object.keys(b).length
    ) {
        return Object.keys(a).every(value => mal_equals(a[value], b[value]));
    } else {
        return false;
    }
}

function mal_apply(fn, ...args) {
    if (typeof fn === 'function')
        return fn(...args);
    if (Array.isArray(fn) && typeof fn[0] === 'function')
        return fn[0](...args);
    throw "Not callable";
}

function quasiquote(ast) {
    if(!Array.isArray(ast) || typeof ast[0] !== 'boolean') {
        return ast && (typeof ast === 'object' || (typeof ast === 'symbol' && !ast.description.startsWith(':')))
            ? [ false, Symbol('quote'), ast]
            : ast;
    }
    if (!ast[0] && typeof ast[1] === 'symbol' && ast[1].description === 'unquote')
        return ast[2];

    const value = ast.slice(1).reduceRight(
        (rest, elt) => {
            if(Array.isArray(elt)
                && elt.length > 1
                && !elt[0]
                && typeof elt[1] === 'symbol'
                && elt[1].description === 'splice-unquote'
            ) return [false, Symbol('concat'), elt[2], rest];
            return [false, Symbol('cons'), quasiquote(elt), rest];
        },
        [false]
    );

    return ast[0] ? [false, Symbol('vec'), value] : value;
}

const ns = {
    '+': (...args) => args.reduce(((a, b) => a + b), 0),
    '-': (...args) => args.reduce((a, b) => a - b ),
    '*': (...args) => args.reduce(((a, b) => a * b), 1),
    '/': (...args) => args.reduce((a, b) => Math.trunc(a / b)),
    'pr-str': (...args) => args.map((str) => pr_str(str, true)).join(' '),
    'str': (...args) => args.map((str) => pr_str(str, false)).join(''),
    'prn': (...args) => { console.log(args.map((str) => pr_str(str, true)).join(' ')); return null; },
    'println': (...args) => { console.log(args.map((str) => pr_str(str, false)).join(' ')); return null; },
    'list': (...args) => [ false, ...args ],
    'list?': list => Array.isArray(list) && !list[0],
    'empty?': list => Array.isArray(list) && typeof list[0] === 'boolean' && list.length < 2,
    'count': list => ((Array.isArray(list) && typeof list[0] === 'boolean') ? list.length - 1 : 0),
    '=': mal_equals,
    '<': (a, b) => a < b,
    '<=': (a, b) => a <= b,
    '>': (a, b) => a > b,
    '>=': (a, b) => a >= b,
    'read-string': read_str,
    'slurp': filename => fs.readFileSync(filename, 'utf-8'),
    'atom': value => ['atom', value],
    'atom?': atom => Array.isArray(atom) && atom[0] === 'atom',
    'deref': atom => ((Array.isArray(atom) && atom[0] === 'atom') ? atom[1] : null),
    'reset!': (atom, value) => {
        if(Array.isArray(atom) && atom[0] === 'atom')
            atom[1]=value;
        return value;
    },
    'swap!': (atom, fn, ...args) => {
        if(!Array.isArray(atom) || atom[0] !== 'atom')
            return null;
        atom[1]=mal_apply(fn, atom[1], ...args);
        return atom[1];
    },
    'apply': mal_apply,
    'cons': (a, b) => ((Array.isArray(b) && typeof b[0] === 'boolean') ? [false, a].concat(b.slice(1)) : [false, a, b]),
    'concat': (...args) => [false].concat(...args.map(arg => arg.slice(1))),
    'vec': value => [true].concat(value.slice(1)),
    'nth': (value, index) => {
        if (Array.isArray(value) &&  typeof value[0] === 'boolean' && index < value.length - 1)
            return value[index+1];
        throw 'index out of range';
    },
    'first': value => ((Array.isArray(value) && typeof value[0] === 'boolean') ? value[1] : null),
    'rest': value => ((Array.isArray(value) && typeof value[0] === 'boolean') ? [false].concat(value.slice(2)) : [false]),
    'throw': value => { throw value; },
    'apply': (fn, ...args) => mal_apply(fn, ...args.concat(args.pop().slice(1))),
    'map': (fn, list) => [false].concat(list.slice(1).map(e => mal_apply(fn, e))),
    'nil?': value => value === null,
    'true?': value => value === true,
    'false?': value => value === false,
    'symbol?': value => typeof value === 'symbol' && !value.description.startsWith(':'),
    'symbol': value => Symbol(value),
    'keyword': value => {
        if (typeof value === 'symbol')
            return value.description.startsWith(':') ? value : Symbol(':' + value.description);
        return Symbol(':' + value);
    },
    'keyword?': value => typeof value === 'symbol' && value.description.startsWith(':'),
    'vector': (...args) => [true, ...args],
    'vector?': value => Array.isArray(value) && value[0] === true,
    'sequential?': value => Array.isArray(value) && typeof value[0] === 'boolean',
    'hash-map': (...args) => {
        const obj = {};
        while(args.length) {
            let key = args.shift();
            key = typeof key === 'symbol' ? key.description : '"' + key;
            obj[key] = args.shift();
        }
        return obj;
    },
    'map?': value => typeof value === 'object' && !Array.isArray(value),
    'assoc': (obj, ...args) => {
        obj = Object.assign({}, obj);
        while(args.length) {
            let key = args.shift();
            key = typeof key === 'symbol' ? key.description : '"' + key;
            obj[key] = args.shift();
        }
        return obj;
    },
    'dissoc': (obj, ...args) => {
        obj = Object.assign({}, obj);
        while(args.length) {
            let key = args.shift();
            key = typeof key === 'symbol' ? key.description : '"' + key;
            delete obj[key];
        }
        return obj;
    },
    'get': (value, key) => {
        key = typeof key === 'symbol' ? key.description : '"' + key;
        return (value && typeof value === 'object' && !Array.isArray(value) && key in value) ? value[key] : null;
    },
    'contains?': (value, key) => {
        key = typeof key === 'symbol' ? key.description : '"' + key;
        return value && typeof value === 'object' && !Array.isArray(value) && key in value;
    },
    'keys': value => [false, ...Object.keys(value).map(k => k.startsWith('"') ? k.slice(1) : Symbol(k))],
    'vals': value => [false, ...Object.values(value)]
};

exports.quasiquote = quasiquote;
exports.repl_env = new Env(null, ns);
