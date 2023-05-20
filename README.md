# SERJSON

A JSON serialization library made in C++.

The implementation is *jbge* (just barely good enough) but the usage
is very simple.

## Introduction

Inside `serjson.h` there is a namespace called `json`
that contains all the necessary things to save your objects inside
a *JSON* file.

Taking inspiration from *Java*, your class/struct needs to
extend `Serializable` abstract class.
You need to override two methods called `serialize()` and `deserialize()`.

## Writing JSON files

Inside the method `json::Node serialize()` you need to construct a `json::Node`
for each attribute of the class.

`json::Node` is an entity that contains a *type*, a *key* and a *value*.
The type is an enumeration and it can contain:
- json::**string**
- json::**number**
- json::**object**
- json::**null**
- json::**array**
- json::**boolean**

There is a distinction between *json::object* and *json::null*
because of the handling of the value inside `serjson.cpp`.

If the desired attribute is an array, you need to construct a node
that contains a *std::vector* of *json::Node* where each node contains
the i-th value of the array and its key is empty.

If you need to construct an object you need as well of a 
*std::vector* of *json::Node* but now each node has to contain
its key.

Obviously this two have different node types.

If you need to create a node from an attribute that contains **null**,
the value is stored as a *std::string*.

When you have successfully created all your nodes, you need to end
the value obtained by calling the function **generate_parent(std::string key, std::vector children)**
placed inside the namespace.
Note: you don't need to create another *std::vector* with your nodes, 
but you can pass them directly to the function as an initializer list.

When you decide to create a file, all you need to do is creating
a `json::Object` and setting a value to *file_path*.
Note: if the specified file does not exist, it will create it for you.

Now you have a representation of the json, but it's empty.
To populate it you simply have to call **add_node(Serializable &obj, Object &json)**
with all the objects you want to save.

When you are ready to save the data, just call 
**write(Object &json)** and it will create/open the file provided
and store your objects inside.

## Reading JSON files

***WORK IN PROGRESS***
