const regexp = /[\s,]*(~@|[\[\]{}()'`~^@]|"(?:\\.|[^\\"])*"?|;.*|[^\s\[\]{}('"`,;)]*)/g;

function tokenize(string) {
    return Array.from(string.matchAll(regexp), m => m[1]);
}

function unquote_string(string) {
    return string.replaceAll(/\\(.?)/g, (str, p1) => {
        if (p1 === '') {
            throw 'Unexpexted EOF';
        }
        return p1 === 'n' ? "\n" : p1
    });
}

const read_macros = {
    '(': (reader) => { return reader.read_list(')'); },
    '[': (reader) => { return reader.read_list(']'); },
    '{': (reader) => { return reader.read_map(); },
    ')': (reader) => { throw 'Unexpected )'; },
    ']': (reader) => { throw 'Unexpected ]'; },
    '}': (reader) => { throw 'Unexpected }'; },
    "'": (reader) => { reader.next(); return [ false, Symbol('quote'), reader.read_form()]; },
    '`': (reader) => { reader.next(); return [ false, Symbol('quasiquote'), reader.read_form()]; },
    '~': (reader) => { reader.next(); return [ false, Symbol('unquote'), reader.read_form()]; },
    '~@': (reader) => { reader.next(); return [ false, Symbol('splice-unquote'), reader.read_form()]; },
    '@': (reader) => { reader.next(); return [ false, Symbol('deref'), reader.read_form()]; },
    '^': (reader) => { reader.next(); const meta = reader.read_form(); return [ false, Symbol('with-meta'), reader.read_form(), meta]; },
}
class Reader {
    tokens;

    constructor(string) {
        this.tokens = tokenize(string);
    }
    next() {
        return this.tokens.shift();
    }
    peek() {
        return this.tokens[0];
    }
    read_form() {
        while(true) {
            const token = this.peek();
            if (token === undefined || token === '') {
                throw 'Unexpected EOF';
            } else if (token in read_macros) {
                return read_macros[token](this);
            } else if(!token.startsWith(';')) {
                return this.read_atom();
            }
            this.next();
        }
    }
    read_list(end) {
        this.next();
        const list = [ end === ']' ];
        while (this.peek() !== end) {
            list.push(this.read_form());
        }
        this.next();
        return list;
    }
    read_map() {
        this.next();
        const map = { };
        while (this.peek() !== '}') {
            const key = this.read_form();
            const value = this.read_form();
            if (typeof key === 'symbol' && key.description.startsWith(':'))  {
                map[key.description] = value;
            } else if (typeof key === 'string') {
                map[key] = value;
            } else {
                throw 'Invalid key in map literal';
            }
        }
        this.next();
        return map;
    }
    read_atom() {
        const token = this.next();
        const num = Number(token);
        if (Number.isInteger(num)) {
            return num;
        } else if (token.startsWith('"')) {
            if (token.length >1 && token.endsWith('"')) {
                return unquote_string(token.slice(1, -1));
            }
            throw 'Unexpected EOF';
        } else if (token === 'nil') {
            return null;
        } else if (token === 'true') {
            return true;
        } else if (token === 'false') {
            return false;
        } else {
            return Symbol(token);
        }
    }
}

module.exports = function (string) {
    return new Reader(string).read_form();
}
