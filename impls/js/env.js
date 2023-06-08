class Env {
    constructor(outer, binds, values) {
        this.outer = outer ? outer : null;
        if (Array.isArray(binds) && typeof binds[0] === 'boolean') {
            this.data={};
            const binds_list = binds.slice(1);
            while (binds_list.length && binds_list[0].description != '&') {
                this.set(binds_list.shift(), values.shift()); 
            }
            if (binds_list.length) {
                this.set(binds_list[1], [false, ...values]);
            }
        } else if (typeof binds === 'object' && !Array.isArray(binds)) {
            this.data = { ...binds };
        } else {
            this.data = {};
        }
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
