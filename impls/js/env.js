class Env {
    constructor(outer) {
        this.outer = outer ? outer : null;
        this.data = {};
    }
    set(key, value) {
        this.data[key.description] = value;
    }
    find(key) {
        if (key.description in this.data) {
            return this;
        } else if (this.outer) {
            return this.outer.find(key);
        } else {
            return null;
        }
    }
    get(key) {
        const env = this.find(key);
        if (env) {
            return env.data[key.description];
        }
        throw key.description + ' not found';
    }
}

module.exports = Env;
