const pr_str = require('./printer.js');
const Env = require('./env.js');

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

const ns = {
    '+': (...args) => { return args.reduce((a, b) => { return a + b }, 0)},
    '-': (...args) => { return args.reduce((a, b) => a - b )},
    '*': (...args) => { return args.reduce((a, b) => { return a * b }, 1)},
    '/': (...args) => { return args.reduce((a, b) => Math.trunc(a / b)); },
    'pr-str': (...args) => { return args.map((str) => pr_str(str, true)).join(' '); },
    'str': (...args) => { return args.map((str) => pr_str(str, false)).join(''); },
    'prn': (...args) => { console.log(args.map((str) => pr_str(str, true)).join(' ')); return null; },
    'println': (...args) => { console.log(args.map((str) => pr_str(str, false)).join(' ')); return null; },
    'list': (...args) => { return [ false, ...args ]; },
    'list?': (list) => { return Array.isArray(list) && !list[0]; },
    'empty?': (list) => { return Array.isArray(list) && list.length < 2; },
    'count': (list) => { return Array.isArray(list) ? list.length - 1 : 0; },
    '=': mal_equals,
    '<': (a, b) => a < b,
    '<=': (a, b) => a <= b,
    '>': (a, b) => a > b,
    '>=': (a, b) => a >= b,
};

exports.repl_env = new Env(null, ns);
