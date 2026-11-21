// The port's smoke program: the smallest thing that proves the C++ runtime is
// really there — <string>, <vector>, <map> and <iostream>, plus a namespace-scope
// object, whose constructor only runs if the loader ran this image's .init_array.
//
// Build:  ninja -C build-meson smoke
// Run:    on a CactOS guest (see PORTING-x86_64.md) — this host cannot map it.

#include <iostream>
#include <map>
#include <string>
#include <vector>

struct Greeting {
    std::string text;
    Greeting() : text("constructed before main") {}
};

static Greeting g_greeting;

int main() {
    std::vector<std::string> words{"libc++", "on", "CactOS"};
    std::map<std::string, int> counts;
    for (const std::string& w : words)
        counts[w] += (int)w.size();

    std::string joined;
    for (const std::string& w : words) {
        if (!joined.empty())
            joined += " ";
        joined += w;
    }

    std::cout << joined << " (" << counts.size() << " distinct words)" << std::endl;
    std::cout << g_greeting.text << std::endl;
    return joined == "libc++ on CactOS" ? 0 : 1;
}
