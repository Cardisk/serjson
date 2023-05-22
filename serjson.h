//
// Created by Matteo Cardinaletti on 11/05/23.
//

#ifndef SERJSON_H
#define SERJSON_H

#include <vector>
#include <string>
#include <variant>
#include <fstream>
#include <ostream>

namespace json {
    // json node type
    typedef enum ObjType {
        empty,
        string,
        number,
        object,
        null,
        array,
        boolean,
    } ObjType;

    // a json element
    typedef struct Node {
        ObjType type = ObjType::empty;
        std::string key;
        std::variant<std::string, float, std::vector<Node>, bool> value;

        template <typename T> T &get_value() {
            return std::get<T>(value);
        }

        Node &operator[] (const std::string &k) {
            static Node empty { .type = ObjType::empty, .key = "", .value = "" };
            if (type != ObjType::object) return empty;

            for (auto& n : get_value<std::vector<Node>>()) {
                if (n.key == k) return n;
            }

            return empty;
        }

        Node &operator[] (const int index) {
            static Node empty { .type = ObjType::empty, .key = "", .value = "" };
            if (type != ObjType::array) return empty;

            if (get_value<std::vector<Node>>().empty()) return empty;

            if (index < 0 || index >= get_value<std::vector<Node>>().size()) return empty;
            return get_value<std::vector<Node>>()[index];
        }

        friend std::ostream &operator<<(std::ostream &os, Node &node) {
            if (!node.key.empty()) os << "{ key: \"" << node.key << "\", value: ";
            switch (node.type) {
                case string:
                    os << node.get_value<std::string>();
                    break;
                case number:
                    os << node.get_value<float>();
                    break;
                case object:
                    {
                        std::vector<Node> v = node.get_value<std::vector<Node>>();
                        os << "{ ";
                        for (int i = 0; i < v.size(); ++i) {
                            if (i > 0) os << ", ";
                            os << v[i];
                        }
                        os << " } ";
                    }
                    break;
                case null:
                    os << node.get_value<std::string>();
                    break;
                case array:
                    {
                        std::vector<Node> v = node.get_value<std::vector<Node>>();
                        os << "[ ";
                        for (int i = 0; i < v.size(); ++i) {
                            if (i > 0) os << ", ";
                            os << v[i];
                        }
                        os << " ] ";
                    }
                    break;
                case boolean:
                    os << std::boolalpha;
                    os << node.get_value<bool>();
                    os << std::noboolalpha;
                    break;
                default:
                    break;
            }
            if (!node.key.empty()) os << "}";
            return os;
        }
    } Node;

    // a json object
    typedef struct Object {
        std::string file_path;
        std::vector<Node> nodes;

        [[nodiscard]] bool empty() const {
            return nodes.empty();
        }

        [[nodiscard]] bool is_array() const {
            return !empty() && nodes[0].key.empty();
        }

        void clear() {
            this->nodes.clear();
        }

        Node &operator[] (const std::string& key) {
            static Node empty { .type = ObjType::empty, .key = "", .value = "" };

            for (auto &n : nodes) {
                if (n.key == key) return n;
            }

            return empty;
        }

        Node &operator[] (const int index) {
            static Node empty { .type = ObjType::empty, .key = "", .value = "" };
            if (nodes.empty()) return empty;

            if (index < 0 || index >= nodes.size()) return empty;
            return nodes[index];
        }

        friend std::ostream &operator<< (std::ostream &os, Object &object) {
            os << "{ file path: \"" << object.file_path << "\", number of nodes: " << object.nodes.size() << " }";
            return os;
        }
    } Object;

    class Serializable {
    public:
        virtual json::Node serialize() = 0;
    };

    // read from file
    Object read(const std::string &path);

    Node generate_parent(std::string key, std::vector<Node> children);

    void add_node(Serializable &obj, Object &json);
    void add_node(Node &obj, Object &json);
    void write(Object &json);
}

#endif //SERJSON_H
