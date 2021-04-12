# vastjson
VastJSON library in C++: structured json with top level cached items for giant JSON files - *using [nlohmann::json](https://www.github.com/nlohmann/json)*

This project emerged from a practical need... see [nlohmann::json Issue 1613](https://github.com/nlohmann/json/issues/1613#issuecomment-816945763).

## Why name it VastJSON?

Names like Big, Large, Huge, were already taken... so we found something BIGGER, LARGER... and FAST! So it's VastJSON :)

## Two Modes (???)

- vast object `{ ... }`
- vast list (TODO) `[ ... ]`

## Run tests and use

```
cd tests && make
```

This is "almost" a Single Header library, named [VastJSON.hpp](./src/vastjson/VastJSON.hpp), just copy it into your project, but remember to copy its only dependency together: [json.hpp](./libs/nlohmann/json.hpp).

If you prefer, you can blend these files together, into a single header file (maybe we can also provide that for future official releases).


## Why is it for?

There exist amazing libraries for JSON in C++, such as nlohmann::json and RapidJSON.

But...

Imagine you have a JSON file with thousands of top-level entries... 
I had a json file in disk with 1.5GB size, and when it got loaded into memory, it surpassed 10GB!

It's not entirely clear to me why these things happen, as overhead of json structure should be tiny, but I needed a solution anyway... and then I share it with you all.

The idea is simple: 

- instead of completely loading JSON structure in memory, user is able to lazy-load only specific entries, thus reducing the burden of the json file processing.
- user can drop JSON structure from memory whenever is wanted, but keeping some string version of it for future re-loads, if necessary
- user can also permanently drop entries, if not intending to use anymore

## Can you give me one example?

Sure. The best example is some situation like this:

```{json}
{
   "A" : {  /* large amount of data here */ },
   "B" : {  /* large amount of data here */ },
   /* thousands of entries here */
   "ZZZZZ..." : {  /* large amount of data here */ }
}
```

In this case, BigJSON would load the structure, while avoiding to parse internal items, only those in top-level.

Example in C++:

```{cpp}
    std::ifstream if_test("demo/test.json");
    vastjson::VastJSON bigj(if_test);
    std::cout << "LOADED #KEYS = " << bigj.size() << std::endl;
```

## Can you give me a BAD example?

Sure.
Imagine you have a SINGLE (or a few) top-level entries, with all the burden inside:

```{json}
{
   "single entry" : {  /* large amount of data here */ }
}
```

Definitely, do NOT use this library, if that's your case.

## How is this implemented?

Currently, it uses the [nice json library from nlohmann](https://github.com/nlohmann/json).

## Known Issues

Right now, this is already used on practice for very large databases!

Anyway, there's a terrible drawback: no internal string can contain "{" or "}". This is easy to fix, but not fixed yet. 

## Usage

Consider JSON:

```{.json}
{
    "A" : { },
    "B" : { 
            "B1":10,
            "B2":"abcd" 
          },
    "Z" : { }
}
```

And file `main.cpp` (see `demo/` folder):

```{.cpp}
    #include <vastjson/VastJSON.hpp>
    #include <iostream>
    // ...

int main() {
    std::ifstream if_test2("demo/test2.json");
    vastjson::VastJSON bigj2(if_test2);
    std::cout << "LOADED #KEYS = " << bigj2.size() << std::endl; // 3
    std::cout << bigj2["A"] << std::endl;
    std::cout << bigj2["B"]["B2"] << std::endl;
    std::cout << bigj2["Z"] << std::endl;
    // ...
}
```

### Lazy loading giant files

If stream file ownership is given to VastJSON, it will only consume necessary parts of JSON.

``` 
{
    "A" : { },
    "B" : { 
            "B1":10,
            "B2":"abcd" 
          },
    "C" : { },
    "Z" : { }
}
```


```
        vastjson::VastJSON bigj3(new std::ifstream("demo/test3.json"));
    // pending operations
    std::cout << "isPending(): " << bigj3.isPending() << std::endl;
    // cache size
    std::cout << "cacheSize(): " << bigj3.cacheSize() << std::endl;
    std::cout << "getUntil(\"\",1) first key is found" << std::endl;
    // get first keys
    bigj3.getUntil("", 1);
    // iterate over top-level keys (cached only!)
    for (auto it = bigj3.begin(); it != bigj3.end(); it++)
        std::cout << it->first << std::endl;
    // direct access will load more
    std::cout << "direct access to bigj3[\"B\"][\"B1\"] = " << bigj3["B"]["B1"] << std::endl;
    // cache size
    std::cout << "cacheSize(): " << bigj3.cacheSize() << std::endl;    
    // still pending
    std::cout << "isPending(): " << bigj3.isPending() << std::endl;
    // iterate over top-level keys (cached only!)
    for (auto it = bigj3.begin(); it != bigj3.end(); it++)
        std::cout << it->first << std::endl;

    // real size (will force performing top-level indexing)
    std::cout << "compute size will force top-level indexing...\nsize(): " << bigj3.size() << std::endl;
    // cache size
    std::cout << "cacheSize(): " << bigj3.cacheSize() << std::endl;    
    // not pending anymore
    std::cout << "isPending(): " << bigj3.isPending() << std::endl;
    // iterate over top-level keys (cached only!)
    for (auto it = bigj3.begin(); it != bigj3.end(); it++)
        std::cout << it->first << std::endl;
```

Output:

```
isPending(): 1
cacheSize(): 0
getUntil("",1) first key is found
A
direct access to bigj3["B"]["B1"] = 10
cacheSize(): 2
isPending(): 1
A
B
compute size will force top-level indexing...
 size(): 4
cacheSize(): 4
isPending(): 0
A
B
C
Z
```

### Build with Bazel

```
bazel build ...
./bazel-bin/app_demo
```

Output should be:
```
LOADED #KEYS = 3
{}
"abcd"
{}
```

### Using Bazel
If you use Bazel Build, just add this into your `WORKSPACE.bazel`:

```
git_repository(
    name='VastJSON',
    remote='https://github.com/igormcoelho/vastjson.git',
    branch='main'
)
```

In `BUILD.bazel`, add this dependency:
```
deps = ["@BigJSON//src/vastjson:vastjson_lib"]
```

Then, just `#include <vastjson/VastJSON.hpp>` in your C++ source files.

## License

MIT License
