# bigjson
BigJSON library in C++: structured json with high level cached items (for giant JSON files)

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
    bigjson::BigJSON bigj(if_test);
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

## Known Issues

Right now, this is already used on practice for very large databases!

Anyway, there's a terrible drawback: no internal string can contain "{" or "}". This is easy to fix, but not fixed yet. 

## Usage

```{cpp}
    std::ifstream if_test("demo/test.json");
    bigjson::BigJSON bigj(if_test);
    std::cout << "LOADED #KEYS = " << bigj.size() << std::endl;
    // more info coming next...
```

## License

MIT License