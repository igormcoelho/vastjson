from src.vastjson_py.VastJSON import VastJSON
vjson = VastJSON("{\"B\":10}")

x = vjson.atCache("B")
print(x)

print(vjson.size())
print(vjson.cacheSize())


vjson2 = VastJSON("tests/testdata/test2.json", 0)
print(vjson2.cacheSize())
print(vjson2.size())
print(vjson2.cacheSize())