const readline = require('readline');

function READ(line) {
    return line;
}
function EVAL(line) {
    return line;
}
function PRINT(line) {
    return line;
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
    console.log(line);
    rl.prompt();
}).on('close', () => {
  process.exit(0);
});
