function quote_string(string) {
    return '"' + string
        .replaceAll('\\', '\\\\')
        .replaceAll("\n", '\\n')
        .replaceAll('"', '\\"') + '"';
}

function pr_str (value, print_readably) {
    if (typeof value === 'symbol') {
        return value.description;
    } else if (Array.isArray(value)) {
       const start = value[0] ? '[' : '(';
       const end = value[0] ? ']' : ')';
       return start + value.slice(1).map((v) => pr_str(v, print_readably)).join(' ') + end;
    } else if (typeof value === 'object') {
        return '{' + Object.keys(value).map((k) =>
            pr_str(k.startsWith(':') ? Symbol(k) : k, print_readably) + ' ' + pr_str(value[k], print_readably)
        ).join(' ') + '}';
    } else if (typeof value === 'string' && print_readably) {
        return quote_string(value);
    } else {
        return value.toString();
    }
}

module.exports = pr_str;
