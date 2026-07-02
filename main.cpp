#include <iostream>
#include <cstdint>
#include <fstream>
#include <chrono>
#include <random>
#include <SDL3/SDL.h>

using namespace std;

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;
uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, 
	0x20, 0x60, 0x20, 0x20, 0x70, 
	0xF0, 0x10, 0xF0, 0x80, 0xF0, 
	0xF0, 0x10, 0xF0, 0x10, 0xF0, 
	0x90, 0x90, 0xF0, 0x10, 0x10, 
	0xF0, 0x80, 0xF0, 0x10, 0xF0, 
	0xF0, 0x80, 0xF0, 0x90, 0xF0, 
	0xF0, 0x10, 0x20, 0x40, 0x40, 
	0xF0, 0x90, 0xF0, 0x90, 0xF0, 
	0xF0, 0x90, 0xF0, 0x10, 0xF0, 
	0xF0, 0x90, 0xF0, 0x90, 0x90, 
	0xE0, 0x90, 0xE0, 0x90, 0xE0, 
	0xF0, 0x80, 0x80, 0x80, 0xF0, 
	0xE0, 0x90, 0x90, 0x90, 0xE0, 
	0xF0, 0x80, 0xF0, 0x80, 0xF0, 
	0xF0, 0x80, 0xF0, 0x80, 0x80  
};
constexpr int VIDEO_WIDTH = 64;
constexpr int VIDEO_HEIGHT = 32;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;
     
class chip8 {
    public:
    uint8_t registers[16]{};
    uint8_t memory[4096]{};
    uint16_t index{}; 
    uint16_t pc{};
    uint16_t stack[16]{};
    uint8_t sp{};
    uint8_t delayTimer{};
    uint8_t soundTimer{};
    uint8_t keypad[16]{};
    uint32_t video[64 * 32]{};
    uint16_t opcode;

    void OP_00E0();
    void OP_00EE();
    void OP_1nnn();
    void OP_2nnn();
    void OP_3xkk();
    void OP_4xkk();
    void OP_5xy0();
    void OP_6xkk();
    void OP_7xkk();
    void OP_8xy0();
    void OP_8xy1();
    void OP_8xy2();
    void OP_8xy3();
    void OP_8xy4();
    void OP_8xy5();
    void OP_8xy6();
    void OP_8xy7();
    void OP_8xyE();
    void OP_9xy0();
    void OP_Annn();
    void OP_Bnnn();
    void OP_Cxkk();
    void OP_Dxyn();
    void OP_Ex9E();
    void OP_ExA1();
    void OP_Fx07();
    void OP_Fx0A();
    void OP_Fx15();
    void OP_Fx18();
    void OP_Fx1E();
    void OP_Fx29();
    void OP_Fx33();
    void OP_Fx55();
    void OP_Fx65();
    void LoadROM(char const* filename);
    void Cycle();

    chip8() : randGen(chrono::system_clock::now().time_since_epoch().count()) {
        
        randByte = uniform_int_distribution<uint8_t> (0, 255U);

        table[0x0] = &chip8::Table0;
        table[0x1] = &chip8::OP_1nnn;
        table[0x2] = &chip8::OP_2nnn;
        table[0x3] = &chip8::OP_3xkk;
        table[0x4] = &chip8::OP_4xkk;
        table[0x5] = &chip8::OP_5xy0;
        table[0x6] = &chip8::OP_6xkk;
        table[0x7] = &chip8::OP_7xkk;
        table[0x8] = &chip8::Table8;
        table[0x9] = &chip8::OP_9xy0;
        table[0xA] = &chip8::OP_Annn;
        table[0xB] = &chip8::OP_Bnnn;
        table[0xC] = &chip8::OP_Cxkk;
        table[0xD] = &chip8::OP_Dxyn;
        table[0xE] = &chip8::TableE;
        table[0xF] = &chip8::TableF;

        for(size_t i = 0; i <= 0xE; i++){
            table0[i] = &chip8::OP_NULL;
            table8[i] = &chip8::OP_NULL;
            tableE[i] = &chip8::OP_NULL;
        }

        table0[0x0] = &chip8::OP_00E0;
        table0[0xE] = &chip8::OP_00EE;

        table8[0x0] = &chip8::OP_8xy0;
        table8[0x1] = &chip8::OP_8xy1;
        table8[0x2] = &chip8::OP_8xy2;
        table8[0x3] = &chip8::OP_8xy3;
        table8[0x4] = &chip8::OP_8xy4;
        table8[0x5] = &chip8::OP_8xy5;
        table8[0x6] = &chip8::OP_8xy6;
        table8[0x7] = &chip8::OP_8xy7;
        table8[0xE] = &chip8::OP_8xyE;

        tableE[0x1] = &chip8::OP_ExA1;
		tableE[0xE] = &chip8::OP_Ex9E;
        
        for(size_t i = 0; i <= 0x65; i++){
            tableF[i] = &chip8::OP_NULL;
        }

        tableF[0x07] = &chip8::OP_Fx07;
        tableF[0x0A] = &chip8::OP_Fx0A;
        tableF[0x15] = &chip8::OP_Fx15;
        tableF[0x18] = &chip8::OP_Fx18;
        tableF[0x1E] = &chip8::OP_Fx1E;
        tableF[0x29] = &chip8::OP_Fx29;
        tableF[0x33] = &chip8::OP_Fx33;
        tableF[0x55] = &chip8::OP_Fx55;
        tableF[0x65] = &chip8::OP_Fx65;

        pc = START_ADDRESS;

        for(unsigned int i=0; i<FONTSET_SIZE; i++) {
            memory[FONTSET_START_ADDRESS + i] = fontset[i];
        }

    }

    void Table0(){
        ((*this).*(table0[opcode & 0x000Fu]))();
    }

    void Table8(){
        ((*this).*(table8[opcode & 0x000Fu]))();
    }

    void TableE()
	{
		((*this).*(tableE[opcode & 0x000Fu]))();
	}

    void TableF()
	{
		((*this).*(tableF[opcode & 0x00FFu]))();
	}

    void OP_NULL(){

    }

    typedef void (chip8::*Chip8Func)();
    Chip8Func table[0xF + 1];
    Chip8Func table0[0xE + 1];
    Chip8Func table8[0xE + 1];
    Chip8Func tableE[0xE + 1];
    Chip8Func tableF[0x65 + 1];

    default_random_engine randGen;
    uniform_int_distribution<uint8_t> randByte;
};


void chip8::LoadROM(char const* filename) {
    ifstream file(filename, ios::binary | ios::ate);

    if(file.is_open()) {
        streampos size = file.tellg();
        char* buffer = new char[size];

        file.seekg(0, ios::beg);
        file.read(buffer, size);    
        file.close();

        for(long i=0; i < size; i++){
            memory[START_ADDRESS + i] = buffer[i];
        }

        delete[] buffer;
    }
}

void chip8::OP_00E0(){
    memset(video, 0, sizeof(video));
}

void chip8::OP_00EE (){
    --sp;
    pc = stack[sp];
}

void chip8::OP_1nnn(){
    uint16_t address = opcode & 0x0FFFu;

    pc = address;
}

void chip8::OP_2nnn(){
    uint16_t address = opcode & 0x0FFFu;

    stack[sp] = pc;
    ++sp;
    pc = address;
}

void chip8::OP_3xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] == byte) {
        pc += 2;
    }
}

void chip8::OP_4xkk(){
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] != byte){
		pc += 2;
	}
}

void chip8::OP_5xy0(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;  

    if(registers[Vx] == registers[Vx]){
        pc += 2;
    }
}

void chip8::OP_6xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = byte;
}

void chip8::OP_7xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] += byte;
}

void chip8::OP_8xy0(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];
}

void chip8::OP_8xy1(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] |= registers[Vy];
}

void chip8::OP_8xy2(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] &= registers[Vy];
}

void chip8::OP_8xy3(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] ^= registers[Vy];
}

void chip8::OP_8xy4(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    uint16_t sum = registers[Vx] + registers[Vy];

    if(sum > 255U) {
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }

    registers[Vx] = sum & 0xFFu;
}

void chip8::OP_8xy5(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vx] >= registers[Vy]) {
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }

    registers[Vx] -= registers[Vy];
}

void chip8::OP_8xy6(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    
    registers[0xF] = (registers[Vx] & 0x1u);
    registers[Vx] >>= 1;
}

void chip8::OP_8xy7(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vy] > registers[Vx]) {
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }

    registers[Vy] = registers[Vy] - registers[Vx];
}

void chip8::OP_8xyE(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

    registers[Vx] <<= 1;
}

void chip8::OP_9xy0(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] != registers[Vy]) {
        pc += 2;
    }
}

void chip8::OP_Annn(){
    uint16_t address = opcode & 0x0FFFu;

    index = address;
}

void chip8::OP_Bnnn(){
    uint16_t address = opcode & 0x0FFFu;

    pc = registers[0] + address;
}

void chip8::OP_Cxkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = randByte(randGen) & byte;
}

void chip8::OP_Dxyn(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t height = opcode & 0x000Fu;

    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;

    for(unsigned int row = 0; row < height; row++) {
        uint8_t spriteByte = memory[index + row];

        for(unsigned int col = 0; col<8; col++){
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            if (spritePixel){
                if (*screenPixel == 0xFFFFFFFF){
                    registers[0xF] = 1;
                }

                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

void chip8::OP_Ex9E(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t key = registers[Vx];

    if (keypad[key]){
        pc += 2;
    }
}

void chip8::OP_ExA1(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t key = registers[Vx];

    if (!keypad[key]){
        pc += 2;
    }
}

void chip8::OP_Fx07(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx] = delayTimer;
}

void chip8::OP_Fx0A(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    if (keypad[0]){
        registers[Vx] = 0;
    } else if (keypad[1]){
        registers[Vx] = 1;  
    } else if (keypad[2]){
        registers[Vx] = 2;  
    } else if (keypad[3]){
        registers[Vx] = 3;  
    } else if (keypad[4]){
        registers[Vx] = 4;  
    } else if (keypad[5]){
        registers[Vx] = 5;  
    } else if (keypad[6]){
        registers[Vx] = 6;  
    } else if (keypad[7]){
        registers[Vx] = 7;  
    } else if (keypad[8]){
        registers[Vx] = 8;  
    } else if (keypad[9]){
        registers[Vx] = 9;  
    } else if (keypad[10]){
        registers[Vx] = 10;  
    } else if (keypad[11]){
        registers[Vx] = 11;  
    } else if (keypad[12]){
        registers[Vx] = 12;  
    } else if (keypad[13]){
        registers[Vx] = 13;  
    } else if (keypad[14]){
        registers[Vx] = 14;  
    } else if (keypad[15]){
        registers[Vx] = 15;  
    } else {
        pc -= 2;  
    }
}

void chip8::OP_Fx15(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    delayTimer = registers[Vx];
}

void chip8::OP_Fx18(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    soundTimer = registers[Vx];
}

void chip8::OP_Fx1E(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    index += registers[Vx];
}

void chip8::OP_Fx29(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t digit = registers[Vx];

    index = FONTSET_START_ADDRESS + (5 * digit);
}

void chip8::OP_Fx33(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = registers[Vx];

    memory[index + 2] = value % 10;
    value /= 10;

    memory[index + 1] = value % 10;
    value /= 10;

    memory[index] = value % 10;
}

void chip8::OP_Fx55(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for(uint8_t i = 0; i <= Vx; i++){
        memory[index + i] = registers[i];   
    }
    index += Vx + 1;
}

void chip8::OP_Fx65(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for(uint8_t i = 0; i <= Vx; i++){
        registers[i] = memory[index + i];
    }
    index += Vx + 1;
}

void chip8::Cycle(){

    opcode = memory[pc] << 8u | memory[pc + 1];
    pc += 2;

    ((*this).*(table[(opcode & 0xF000u) >> 12u]))();

    static auto lastTimerTime = chrono::high_resolution_clock::now();
    auto currentTimerTime = chrono::high_resolution_clock::now();

    float elapsedMs = chrono::duration_cast<chrono::duration<float, milli>>(currentTimerTime - lastTimerTime).count();

    if (elapsedMs >= 16.67f){
        if(delayTimer > 0){
            --delayTimer;
        }
        if(soundTimer > 0){
            --soundTimer;
        }
        lastTimerTime = currentTimerTime;
    }
}

class Platform{
    public:
    Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight){
        if(!SDL_Init(SDL_INIT_VIDEO)){
            cerr << "SDL could not initialize! SDL error: " << SDL_GetError() << '\n'; 
            SDL_Quit();
        }
   
        SDL_CreateWindowAndRenderer(title, windowWidth, windowHeight, 0, &window, &renderer);

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);

    }

    ~Platform(){
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void Update(void const* buffer, int pitch){
        SDL_UpdateTexture(texture, nullptr, buffer, pitch);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    bool ProcessInput(uint8_t* keys){
        bool quit = false;

        SDL_Event event;

        while(SDL_PollEvent(&event)){
            switch (event.type){
                case SDL_EVENT_QUIT:{
                    quit = true;
                } break;

                case SDL_EVENT_KEY_DOWN:{
                    switch (event.key.key){
                        case SDLK_ESCAPE:{
                            quit = true;
                        } break;

                        case SDLK_X:{
                            keys[0] = 1;
                        } break;

                        case SDLK_1:{
                            keys[1] = 1;
                        } break;

                        case SDLK_2:{
                            keys[2] = 1;
                        } break;    

                        case SDLK_3:{
                            keys[3] = 1;
                        } break;

                        case SDLK_Q:{
                            keys[4] = 1;
                        } break;

                        case SDLK_W:{
                            keys[5] = 1;
                        } break;

                        case SDLK_E:{
                            keys[6] = 1;
                        } break;

                        case SDLK_A:{
                            keys[7] = 1;
                        } break;

                        case SDLK_S:{
                            keys[8] = 1;
                        } break;

                        case SDLK_D:{
                            keys[9] = 1;
                        } break;

                        case SDLK_Z:{
                            keys[0xA] = 1;
                        } break;

                        case SDLK_C:{
                            keys[0xB] = 1;
                        } break;

                        case SDLK_4:{
                            keys[0xC] = 1;
                        } break;

                        case SDLK_R:{
                            keys[0xD] = 1;
                        } break;

                        case SDLK_F:{
                            keys[0xE] = 1;
                        } break;

                        case SDLK_V:{
                            keys[0xF] = 1;
                        } break;
                    }
                } break;

                case SDL_EVENT_KEY_UP:{
                    switch (event.key.key){
                        case SDLK_X:{
                            keys[0] = 0;
                        } break;

                        case SDLK_1:{
                            keys[1] = 0;
                        } break;

                        case SDLK_2:{
                            keys[2] = 0;
                        } break;    

                        case SDLK_3:{
                            keys[3] = 0;
                        } break;

                        case SDLK_Q:{
                            keys[4] = 0;
                        } break;

                        case SDLK_W:{
                            keys[5] = 0;
                        } break;

                        case SDLK_E:{
                            keys[6] = 0;
                        } break;

                        case SDLK_A:{
                            keys[7] = 0;
                        } break;

                        case SDLK_S:{
                            keys[8] = 0;
                        } break;

                        case SDLK_D:{
                            keys[9] = 0;
                        } break;

                        case SDLK_Z:{
                            keys[0xA] = 0;
                        } break;

                        case SDLK_C:{
                            keys[0xB] = 0;
                        } break;

                        case SDLK_4:{
                            keys[0xC] = 0;
                        } break;

                        case SDLK_R:{
                            keys[0xD] = 0;
                        } break;

                        case SDLK_F:{
                            keys[0xE] = 0;
                        } break;

                        case SDLK_V:{
                            keys[0xF] = 0;
                        } break;
                    }
                } break;
            }
        }

        return quit;
    }

    private:
        SDL_Window* window{};
        SDL_Renderer* renderer{};
        SDL_Texture* texture{};
};

int main(int argc, char* argv[]){
    if (argc != 4){
        cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
        exit(EXIT_FAILURE);
    }

    int videoScale = stoi(argv[1]);
    int cycleDelay = stoi(argv[2]);
    char const* romFilename = argv[3];

    Platform platform("CHIP-8 Emulator", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);

    chip8 Chip8;
    Chip8.LoadROM(romFilename);

    int videoPitch = sizeof(Chip8.video[0]) * VIDEO_WIDTH;

    auto lastCycleTime = chrono::high_resolution_clock::now();
    bool quit = false;

    while (!quit){
        auto currentTime = chrono::high_resolution_clock::now();
        float dt = chrono::duration_cast<chrono::duration<float, std::milli>>(currentTime - lastCycleTime).count();
        
        if (dt >= cycleDelay){
            lastCycleTime = currentTime;
            quit = platform.ProcessInput(Chip8.keypad);
            Chip8.Cycle();
            platform.Update(Chip8.video, videoPitch);
        }
    }
    return 0;
}