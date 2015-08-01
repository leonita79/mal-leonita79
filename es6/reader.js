import * as types from './types';

export class BlankException extends Error {}

class Reader {
    constructor(tokens) {
        this.tokens = tokens;
        this.position = 0;
    }
    next() { return this.tokens[this.position++]; }
    peek() { return this.tokens[this.position]; }
}

function tokenize(str) {
    const re = /[\s,]*(~@|[\[\]{}()'`~^@]|"(?:\\.|[^\\"])*"|;.*|[^\s\[\]{}('"`,;)]*)/g;
    let match = null;
    let results = [];
    while ((match = re.exec(str)[1]) != '') {
        if (match[0] === ';') { continue; }
        results.push(match);
    }
    return results;
}

function read_atom (reader) {
    const token = reader.next();
    //console.log("read_atom:", token);
    if (token.match(/^-?[0-9]+$/)) {
        return parseInt(token,10)        // integer
    } else if (token.match(/^-?[0-9][0-9.]*$/)) {
        return parseFloat(token,10);     // float
    } else if (token[0] === "\"") {
        return token.slice(1,token.length-1)
            .replace(/\\"/g, '"')
            .replace(/\\n/g, "\n"); // string
    } else if (token[0] === ":") {
        return types._keyword(token.slice(1));
    } else if (token === "nil") {
        return null;
    } else if (token === "true") {
        return true;
    } else if (token === "false") {
        return false;
    } else {
        return types._symbol(token); // symbol
    }
}

// read list of tokens
function read_list(reader, start, end) {
    start = start || '(';
    end = end || ')';
    var ast = [];
    var token = reader.next();
    if (token !== start) {
        throw new Error("expected '" + start + "'");
    }
    while ((token = reader.peek()) !== end) {
        if (!token) {
            throw new Error("expected '" + end + "', got EOF");
        }
        ast.push(read_form(reader));
    }
    reader.next();
    return ast;
}

function read_form(reader) {
    var token = reader.peek();
    switch (token) {
    // reader macros/transforms
    case ';': return null; // Ignore comments
    case '\'': reader.next();
               return [types._symbol('quote'), read_form(reader)];
    case '`': reader.next();
              return [types._symbol('quasiquote'), read_form(reader)];
    case '~': reader.next();
              return [types._symbol('unquote'), read_form(reader)];
    case '~@': reader.next();
               return [types._symbol('splice-unquote'), read_form(reader)];
    case '^': reader.next();
              var meta = read_form(reader);
              return [types._symbol('with-meta'), read_form(reader), meta];
    case '@': reader.next();
              return [types._symbol('deref'), read_form(reader)];

    // list
    case ')': throw new Error("unexpected ')'");
    case '(': return read_list(reader);

    // atom
    default:  return read_atom(reader);
    }
}

export function read_str(str) {
    var tokens = tokenize(str);
    if (tokens.length === 0) { throw new BlankException(); }
    return read_form(new Reader(tokens))
}
