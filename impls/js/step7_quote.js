const readline = require('readline');
const read_str = require('./reader.js');
const pr_str = require('./printer.js');
const Env = require('./env.js');
const core = require('./core.js');

const special = {
    'def!': (context) => {
        const name = context.ast[2];
        context.ast = EVAL(context.ast[3], context.env);
        context.env.set(name, context.ast);
        return true;
    },
    'let*': (context) => {
        context.env = new Env(context.env);
        const bindings = context.ast[2].slice(1);
        while(bindings.length) {
            const name = bindings.shift();
            context.env.set(name, EVAL(bindings.shift(), context.env));
        }
        context.ast = context.ast[3];
    },
    'do': (context) => {
        const code = context.ast.slice(2);
        while (code.length > 1) {
            EVAL(code.shift(), context.env);
        }
        context.ast = code.shift();
    },
    'if': (context) => {
        const value = EVAL(context.ast[2], context.env);
            if (value == null || value === false) { //null, undefined, or false
            context.ast = context.ast[4];
        } else {
            context.ast = context.ast[3];
        }
    },
    'fn*': (context) => {
        const binds = context.ast[2];
        const code=context.ast[3];
        const env = context.env;

        context.ast = [
            (...args) => {
                return EVAL(code, new Env(env, binds, args));
            },
            code,
            binds,
            env
        ];
        return true;
    },
    'quote': (context) => {
        context.ast = context.ast[2];
        return true;
    },
    'quasiquote': (context) => {
        context.ast = core.quasiquote(context.ast[2]);
    },
    'quasiquoteexpand': (context) => {
        context.ast = core.quasiquote(context.ast[2]);
        return true;
    }
};

function READ(line) {
    return read_str(line);
}

function eval_ast(ast, env) {
    if (typeof ast === 'symbol' && !ast.description.startsWith(':')) {
        return env.get(ast);
    } else if (Array.isArray(ast) && typeof ast[0] === 'boolean') {
        return ast.map((value) => EVAL(value, env));
    } else if (typeof ast === 'object' && ast !== null) {
        return Object.fromEntries(
            Object.entries(ast)
            .map(([key, value]) => [key, EVAL(value, env)])
        );
    } else {
        return ast;
    }
}

function EVAL(ast, env) {
    const context= {
        'ast': ast,
        'env': env
    };

    while(true) {
        if (!Array.isArray(context.ast) || context.ast[0]) {
            return eval_ast(context.ast, context.env);
        } else if (context.ast.length < 2) {
            return context.ast;
        } else if (typeof context.ast[1] === 'symbol' && context.ast[1].description in special) {
            if (special[context.ast[1].description](context)) {
                return context.ast;
            }
        } else {
            const evaluated_ast = eval_ast(context.ast, context.env);
            if (typeof evaluated_ast[1] === 'function') { 
                return evaluated_ast[1](...evaluated_ast.slice(2));
            } else if (Array.isArray(evaluated_ast[1]) && typeof evaluated_ast[1][0] === 'function') {
                context.ast = evaluated_ast[1][1];
                context.env = new Env(evaluated_ast[1][3], evaluated_ast[1][2], evaluated_ast.slice(2));
            } else {
                throw "Not callable"
            }
        }
    }
}
function PRINT(value) {
    return pr_str(value, true);
}

function rep(line) {
    return PRINT(EVAL(READ(line), core.repl_env));
}

core.repl_env.set(Symbol('eval'), ast => EVAL(ast, core.repl_env));

rep('(def! not (fn* (a) (if a false true)))');
rep('(def! load-file (fn* (f) (eval (read-string (str "(do " (slurp f) "\nnil)")))))')

core.repl_env.set(
    Symbol('*ARGV*'),
    [false].concat(process.argv.slice(3))
);

if (process.argv.length > 2) {
    rep(`(load-file "${process.argv[2]}")`);
    process.exit();
}

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
  prompt: 'user> '
});

rl.prompt();

rl.on('line', (line) => {
    try {
        if(line.trim()) {
            console.log(rep(line));
        }    
    } catch (e) {
        console.log(e);
    }
    rl.prompt();
}).on('close', () => {
  process.exit(0);
});
