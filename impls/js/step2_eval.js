const readline = require('readline');
const read_str = require('./reader.js');
const pr_str = require('./printer.js');
const repl_env = {
    '+': (a, b) => { return a + b; },
    '-': (a, b) => { return a - b; },
    '*': (a, b) => { return a * b; },
    '/': (a, b) => { return Math.trunc(a / b); },
};

function READ(line) {
    return read_str(line);
}

function eval_ast(ast, env) {
    if (typeof ast === 'symbol' && !ast.description.startsWith(':')) {
        if (ast.description in env) {
            return env[ast.description];
        }
        throw ast.description + ' is undefined'; 
    } else if (Array.isArray(ast) && typeof ast[0] === 'boolean') {
        return ast.map((value) => EVAL(value, env));
    } else if (typeof ast === 'object') {
        return Object.fromEntries(
            Object.entries(ast)
            .map(([key, value]) => [key, EVAL(value, env)])
        );
    } else {
        return ast;
    }
}

function EVAL(ast, env) {
    if (!Array.isArray(ast) || ast[0]) {
        return eval_ast(ast, env);
    } else if (ast.length < 2) {
        return ast;
    }
    const evaluated_ast = eval_ast(ast, env);
    return evaluated_ast[1](...evaluated_ast.slice(2));
}
function PRINT(value) {
    return pr_str(value, true);
}

function rep(line) {
    return PRINT(EVAL(READ(line), repl_env));
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
