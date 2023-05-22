#include <utility>
#include <vector>
#include <iostream>
#include "serjson.h"

class Inner : public json::Serializable {
public:
    json::Node serialize() override {
        json::Node node;
        node.type = json::number;
        node.key = "x";
        node.value = static_cast<float>(x);

        // to generate a node, maybe is better to choose some sort of unique key.
        return json::generate_parent("inner", {node});
    }

private:
    int x = 13;
};

class Example : public json::Serializable {
public:
    explicit Example(float a, std::string b, std::vector<bool> c, Inner d) : a(a),
                                                                             b(std::move(b)),
                                                                             c(std::move(c)),
                                                                             d(std::move(d)) {}

    explicit Example() : Example(5.f, "example test", {true, false}, Inner()) {}

    // override from the namespace.
    json::Node serialize() override {
        json::Node n_a;
        n_a.type = json::number;
        n_a.key = "a";
        n_a.value = a;

        json::Node n_b;
        n_b.type = json::string;
        n_b.key = "b";
        n_b.value = b;

        // TODO: find a better way to create array nodes
        json::Node n_c;
        n_c.type = json::array;
        n_c.key = "c";
        std::vector<json::Node> nodes;
        for (bool i : c) {
            json::Node n;
            n.type = json::boolean;
            n.key = "";
            n.value = i;
            nodes.push_back(n);
        }
        n_c.value = nodes;

        // TODO: find a better way to create object nodes
        json::Node n_d;
        n_d.type = json::object;
        n_d.key = "d";
        n_d.value = d.serialize().get_value<std::vector<json::Node>>();

        // to generate a node, maybe is better to choose some sort of unique key.
        return json::generate_parent("example", {n_a, n_b, n_c, n_d});
    }

private:
    float a;
    std::string b;
    std::vector<bool> c;
    Inner d;
};

int main() {
    // calling the default constructor.
    Example e;

    // creating a runtime representation object.
    json::Object json1;
    // specifying the file where to store the data.
    json1.file_path = "../example.json";

    // manually adding the desired nodes to the object.
    json::add_node(e, json1);

    // calling the json::write directive.
    json::write(json1);

    // recreating the runtime representation by reading the file.
    json::Object json2 = json::read(json1.file_path);

    // outputting something to see if it is correct.
    std::cout << json2["example"]["c"];
    return 0;
}
