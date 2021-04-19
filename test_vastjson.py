from src.vastjson_py.VastJSON import VastJSON
vjson = VastJSON("{\"B\":10}")

x = vjson.atCache("B")
print(x)

print(vjson.size())
print(vjson.cacheSize())
