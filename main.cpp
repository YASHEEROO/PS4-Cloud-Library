#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <regex>
#include <vector>
#include <string>
#include <iostream>

using json = nlohmann::json;
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

struct GameAsset {
    std::string title;
    std::string platform;
    std::string pkgUrl;
    std::string imageUrl; // رابط الصورة السحابي التلقائي
    SDL_Texture* posterTexture = nullptr;
};

// --- [🌐 نظام الذاكرة المؤقتة لـ cURL] ---
struct MemoryBuffer {
    std::vector<unsigned char> data;
};

size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    MemoryBuffer* mem = (MemoryBuffer*)userp;
    unsigned char* ptr = (unsigned char*)contents;
    mem->data.insert(mem->data.end(), ptr, ptr + realsize);
    return realsize;
}

// --- [🖼️ دالة تنزيل الصور سحابياً وتحويلها فوراً إلى Texture] ---
SDL_Texture* downloadCloudTexture(const std::string& imageUrl, SDL_Renderer* renderer) {
    CURL* curl = curl_easy_init();
    if (!curl) return nullptr;

    MemoryBuffer chunk;
    curl_easy_setopt(curl, CURLOPT_URL, imageUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 6L); // مهلة 6 ثوانٍ للتحميل
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || chunk.data.empty()) return nullptr;

    SDL_RWops* rw = SDL_RWFromMem(chunk.data.data(), chunk.data.size());
    if (!rw) return nullptr;

    SDL_Surface* surface = IMG_Load_RW(rw, 1);
    if (!surface) return nullptr;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

// --- [🛡️ نظام الأمان وفحص الروابط] ---
bool isValidURLStructure(const std::string& url) {
    const std::regex urlRegex(R"(^https?://([^/\s]+)(/[^\s]*)?$)");
    return std::regex_match(url, urlRegex);
}

bool isLinkAlive(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    long httpCode = 0;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 4L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    
    if (curl_easy_perform(curl) == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    }
    curl_easy_cleanup(curl);
    return (httpCode == 200 || httpCode == 206 || httpCode == 302);
}

// --- [📡 نظام الحقن والتنزيل في الـ PS4] ---
bool triggerPS4Download(const std::string& pkgUrl) {
    int targetSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (targetSocket < 0) return false;

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12800); 
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    if (connect(targetSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        close(targetSocket);
        return false;
    }

    std::string jsonPayload = "{\"type\": \"direct\", \"packages\": [\"" + pkgUrl + "\"]}";
    std::string httpRequest = "POST /api/install HTTP/1.1\r\nHost: 127.0.0.1:12800\r\nContent-Type: application/json\r\nContent-Length: " + std::to_string(jsonPayload.length()) + "\r\n\r\n" + jsonPayload;

    send(targetSocket, httpRequest.c_str(), httpRequest.length(), 0);
    close(targetSocket);
    return true;
}

// --- [🎮 محرك التطبيق الرئيسي] ---
int main(int argc, char* args) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) return -1;
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    curl_global_init(CURL_GLOBAL_ALL);

    SDL_Window* window = SDL_CreateWindow("PS3/PS4 Cloud Store", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    TTF_Font* fontTitle = TTF_OpenFont("/app0/data/font.ttf", 26);
    TTF_Font* fontBadge = TTF_OpenFont("/app0/data/font.ttf", 18);

    // 📋 قاعدة البيانات المستضافة بروابط الصور المطابقة والجاهزة للسحب التلقائي
    std::string rawJson = R"({
        "games": [
            {
                "title": "God of War III Remastered",
                "platform": "PS4",
                "pkg_url": "http://192.168.1",
                "img_url": "https://steamstatic.com"
            },
            {
                "title": "The Last of Us Remastered",
                "platform": "PS4",
                "pkg_url": "http://192.168.1",
                "img_url": "https://steamstatic.com"
            },
            {
                "title": "Bloodborne",
                "platform": "PS4",
                "pkg_url": "http://192.168.1",
                "img_url": "https://igdb.com"
            },
            {
                "title": "Uncharted 3: Drake's Deception",
                "platform": "PS3",
                "pkg_url": "https://archive.org",
                "img_url": "https://igdb.com"
            }
        ]
    })";

    auto parsed = json::parse(rawJson);
    std::vector<GameAsset> library;
    for (const auto& item : parsed["games"]) {
        GameAsset game = {item["title"], item["platform"], item["pkg_url"], item["img_url"]};
        
        // 🔄 سحب تنزيل الصورة تلقائياً من الخادم عند تشغيل الصفحة
        std::cout << "[🌐 سحابي] جاري تنزيل بوستر: " << game.title << "...\n";
        game.posterTexture = downloadCloudTexture(game.imageUrl, renderer);
        
        library.push_back(game);
    }

    int currentIndex = 0;
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) isRunning = false;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_RIGHT: if (currentIndex < library.size() - 1) currentIndex++; break;
                    case SDLK_LEFT:  if (currentIndex > 0) currentIndex--; break;
                    case SDLK_RETURN: // تثبيت عند الضغط على Enter أو زر X
                        if (isValidURLStructure(library[currentIndex].pkgUrl) && isLinkAlive(library[currentIndex].pkgUrl)) {
                            triggerPS4Download(library[currentIndex].pkgUrl);
                            std::cout << "[🎉] تم إرسال اللعبة للتحميل بنجاح!\n";
                        } else {
                            std::cout << "[❌] الرابط تالف أو السيرفر غير مستجيب!\n";
                        }
                        break;
                    case SDLK_ESCAPE: isRunning = false; break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 8, 20, 45, 255);
        SDL_RenderClear(renderer);

        // حسابات الواجهة الرسومية وعرض كروت الألعاب المحدثة
        int cardWidth = 320; int cardHeight = 220; int spacing = 40;
        int startX = SCREEN_WIDTH / 2 - (cardWidth / 2) - (currentIndex * (cardWidth + spacing));

        for (int i = 0; i < library.size(); ++i) {
            SDL_Rect cardRect = { startX + (i * (cardWidth + spacing)), 400, cardWidth, cardHeight };
            bool isSelected = (i == currentIndex);
            if (isSelected) { cardRect.x -= 15; cardRect.y -= 15; cardRect.w += 30; cardRect.h += 30; }

            // إذا نجح السحب السحابي التلقائي للصورة يتم رسمها، وإلا يتم رسم كرت افتراضي رمادي
            if (library[i].posterTexture) SDL_RenderCopy(renderer, library[i].posterTexture, NULL, &cardRect);
            else { SDL_SetRenderDrawColor(renderer, 50, 55, 65, 255); SDL_RenderFillRect(renderer, &cardRect); }

            // [🛡️ خط تمييز نوع الجهاز الصغير أسفل الكرت مباشرة]
            SDL_Rect badgeRect = { cardRect.x, cardRect.y + cardRect.h - 30, cardRect.w, 30 };
            if (library[i].platform == "PS4") SDL_SetRenderDrawColor(renderer, 0, 102, 204, 230); // أزرق سوني
            else SDL_SetRenderDrawColor(renderer, 204, 0, 0, 230); // أحمر ريترو للـ PS3
            SDL_RenderFillRect(renderer, &badgeRect);

            if (fontBadge) {
                SDL_Surface* bSurf = TTF_RenderUTF8_Blended(fontBadge, library[i].platform.c_str(), {255,255,255,255});
                SDL_Texture* bTex = SDL_CreateTextureFromSurface(renderer, bSurf);
                SDL_Rect tPos = { badgeRect.x + 15, badgeRect.y + (badgeRect.h/2) - (bSurf->h/2), bSurf->w, bSurf->h };
                SDL_RenderCopy(renderer, bTex, NULL, &tPos);
                SDL_FreeSurface(bSurf); SDL_DestroyTexture(bTex);
            }
            if (isSelected) { SDL_SetRenderDrawColor(renderer, 0, 255, 200, 255); SDL_RenderDrawRect(renderer, &cardRect); }
        }

        // عرض اسم اللعبة المحددة أسفل الواجهة بخط كبير
        if (fontTitle && !library.empty()) {
            SDL_Surface* tSurf = TTF_RenderUTF8_Blended(fontTitle, library[currentIndex].title.c_str(), {255,255,255,255});
            SDL_Texture* tTex = SDL_CreateTextureFromSurface(renderer, tSurf);
            SDL_Rect tPos = { SCREEN_WIDTH / 2 - (tSurf->w / 2), 700, tSurf->w, tSurf->h };
            SDL_RenderCopy(renderer, tTex, NULL, &tPos);
            SDL_FreeSurface(tSurf); SDL_DestroyTexture(tTex);
