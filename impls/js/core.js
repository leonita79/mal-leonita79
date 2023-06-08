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
        return Object.keys(a).every((value, index) => mal_equals(value, b[index]));
    } else {
        return false;
    }
}

function mal_apply(fn, ...args) {
    if (typeof fn === 'function')
        return fn.apply(null, args);
    if (Array.isArray(fn) && typeof fn[0] === 'function')
        return fn[0].apply(null, args);
    throw "Not callable";
}

function quasiquote(ast) {
    if(!Array.isArray(ast)) {
        return ast && typeof ast === 'object' || typeof ast === 'symbol'
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
        []
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
    'empty?': list => Array.isArray(list) && list.length < 2,
    'count': list => (Array.isArray(list) ? list.length - 1 : 0),
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
    'cons': (a, b) => (Array.isArray(b) ? [false, a].concat(b.slice(1)) : [false, a, b]),
    'concat': (...args) => Array.prototype.concat.apply([false], args.map(arg => arg.slice(1))),
    'vec': value => [true].concat(value.slice(1)),
};

exports.quasiquote = quasiquote;
exports.repl_env = new Env(null, ns);
