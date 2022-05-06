const readline = require('readline');
const read_str = require('./reader.js');
const pr_str = require('./printer.js');

function READ(line) {
    return read_str(line);
}
function EVAL(value) {
    return value;
}
function PRINT(value) {
    return pr_str(value, true);
}
function repl(line) {
    return PRINT(EVAL(READ(line)));
}

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
  prompt: 'user> '
});

rl.prompt();

rl.on('line', (line) => {
    try {
        console.log(repl(line));
    } catch (e) {
        console.log(e);
    }
    rl.prompt();
}).on('close', () => {
  process.exit(0);
});
