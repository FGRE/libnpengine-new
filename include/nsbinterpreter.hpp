/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2013-2014 Mislav Blažević <krofnica996@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * */
#ifndef NSB_INTERPRETER_HPP
#define NSB_INTERPRETER_HPP

#include "nsbfile.hpp"

#include <stack>
#include <cstdint>
#include <list>
#include <map>
#include <queue>
#include <vector>
#include <functional>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Clock.hpp>
using std::string;

#define SPECIAL_POS_NUM 7

namespace sf
{
    class RenderTexture;
    class Texture;
}

class NsbFile;
class Game;
class ResourceMgr;
class Line;
class Drawable;
class DrawableBase;
class ArrayVariable;
class Text;
class Playable;
class Phone;

typedef std::vector<std::pair<string, ArrayVariable>> ArrayMembers;

// Represents Nitroscript variable
struct Variable
{
    Variable() : Type("STRING") {}
    Variable(const string& Type, const string& Value) : Type(Type), Value(Value) {}

    string Type;
    string Value;
};

// Note that this structure is actually a tree. See: ArrayMembers typedef
struct ArrayVariable : Variable
{
    ArrayVariable() : Variable() {}
    ArrayVariable(const Variable& Var) : Variable(Var.Type, Var.Value) {}
    ArrayMembers Members;
};

// Element of call stack
// Contains script and line to which function will return upon ending
struct FuncReturn
{
    NsbFile* pScript;
    uint32_t SourceLine;
};

// Interpreter thread context
struct NsbContext
{
    NsbContext();

    string Identifier; // Thread name
    bool Active; // If true, code in this context is exected
    Line* pLine; // Line of code which is currently being executed
    NsbFile* pScript; // Script which is currently being executed (top of call stack)
    sf::Clock SleepClock; // SleepTime clock
    sf::Time SleepTime; // How long should interpreter wait before executing next line
    bool BranchCondition; // If false, code block after If/While is not executed
    std::stack<FuncReturn> Returns; // Call stack

    bool PrevLine(); // Prev instruction
    bool NextLine(); // Next instruction
    void Sleep(int32_t ms);

    bool CallSubroutine(NsbFile* pDestScript, const char* Symbol, SymbolType Type); // Attempts to call specified symbol in specified script
    void ReturnSubroutine(); // Function return: pop the call stack
};

class NsbInterpreter
{
    typedef void (NsbInterpreter::*BuiltinFunc)();
    friend void NitroscriptMain(NsbInterpreter* pInterpreter);
public:
    NsbInterpreter(Game* pGame);
    ~NsbInterpreter();

    void Stop();
    void Pause();
    void Start();

    void CallScript(const string& FileName, const string& Symbol, SymbolType Type);
    void DumpState();
    void PhoneToggle();
private:
    void Run();
    void Sleep(int32_t ms);
    void LoadScript(const string& FileName);
    void ExecuteScript(const string& InitScript);
    void ExecuteScriptLocal(const string& InitScript);

    // Magic handlers
    void CreateThread();
    void LoadTextureClip();
    void Increment();
    void LogicalGreater();
    void LogicalLess();
    void ArraySize();
    void SetParam();
    void Get();
    void DrawToTexture();
    void CreateColor();
    void Fade();
    void Begin();
    void End();
    void LoadTexture();
    void Delete();
    void Call();
    void Add();
    void Format();
    void CreateMovie();
    void DrawTransition();
    void ClearParams();
    void CreateTexture();
    void GetMovieTime();
    void ApplyBlur();
    void CreateWindow();
    void SetTextboxAttributes();
    void LoadAudio();
    void SetAudioRange();
    void SetFontAttributes();
    void SetVolume();
    void DisplayText();
    void Move();
    void Wait();
    void SetLoop();
    void ParseText();
    void Request();
    void RegisterCallback();
    void ArrayRead();
    void Set();
    void Negative();
    void PlaceholderParam();
    void Zoom();
    void If();
    void While();
    void LogicalNot();
    void LogicalEqual();
    void LogicalNotEqual();
    void CallChapter();
    void Center();
    void System();
    void CreateScrollbar();
    void CallScene();
    void Time();
    void Shake();
    void GetScriptName();
    void ScopeBegin();
    void ScopeEnd();

    // Stubs
    void UNK1();
    void UNK2();
    void LoopJump();
    void UNK77();

    // Builtin functions
    void NSBDelete();
    void NSBZoom(Drawable* pDrawable, int32_t Time, float x, float y, const string& Tempo, bool Wait);
    void NSBArrayRead(int32_t Depth);
    void NSBRequest(const string& State);
    void NSBSetLoop(Playable* pMusic, bool Loop);
    void NSBMove(DrawableBase* pDrawable, int32_t Time, int32_t x, int32_t y, const string& Tempo, bool Wait);
    void NSBDisplayText(Text* pText, const string& unk);
    void NSBSetVolume(Playable* pMusic, int32_t NumSeconds, int32_t Volume, const string& Tempo);
    void NSBSetAudioRange(Playable* pMusic, int32_t Begin, int32_t End);
    void NSBSetFontAttributes(const string& Font, int32_t Size, const string& Color1, const string& Color2, int32_t unk0, const string& unk1);
    void NSBLoadAudio(const string& Type, const string& File);
    void NSBSetTextboxAttributes(int32_t unk0, const string& Font, int32_t unk1, const string& Color1, const string& Color2, int32_t unk2, const string& unk3);
    void NSBCreateWindow(int32_t unk0, int32_t x, int32_t y, int32_t Width, int32_t Height, bool unk1);
    void NSBGetMovieTime();
    void NSBFade(DrawableBase* pDrawable, int32_t Time, int32_t Opacity, const string& Tempo, bool Wait);
    void NSBBindIdentifier();
    void NSBCreateArray();
    void NSBCreateThread(int32_t unk1, int32_t unk2, int32_t unk3, const string& Function);
    void NSBSystem(string Command, string Parameters, string Directory);

    // GL functions are builtins like NSB, but need to be called from OpenGL thread
    // See: Game::GLCallback
    void GLDelete(DrawableBase* pDrawable);
    void GLLoadTexture(int32_t Priority, int32_t x, int32_t y, const string& File);
    void GLCreateColor(int32_t Priority, int32_t x, int32_t y, int32_t Width, int32_t Height, string Color);
    // NOTE: Chaos;Head doesn't have last parameter (音声同期)
    void GLCreateMovie(int32_t Priority, int32_t x, int32_t y, bool Loop, bool Alpha, const string& File, bool Audio);
    void GLDrawTransition(Drawable* pDrawable, int32_t Time, int32_t Start, int32_t End, int32_t Range, const string& Tempo, string File, bool Wait);
    void GLCreateTexture(int32_t Width, int32_t Height, const string& Color);
    void GLDrawToTexture(sf::RenderTexture* pTexture, int32_t x, int32_t y, const string& File);
    void GLApplyBlur(Drawable* pDrawable, const string& Heaviness);
    void GLParseText(const string& Box, const string& XML);
    void GLLoadTextureClip(int32_t Priority, int32_t x, int32_t y, int32_t tx, int32_t ty, int32_t width, int32_t height, const string& File);

    // Steins gate hardcoded functions
    Phone* pPhone;
    void SGPhoneOpen();
    void SGPhoneMode();
    void SGPhonePriority();

    template <class T> T GetParam(int32_t Index); // If parameter is identifier, it is transformed to value
    template <class T> T GetVariable(const string& Identifier); // Transforms identifier to value
    template <class T> void WildcardCall(std::string Handle, std::function<void(T*)> Func); // Calls Func for all handles matching wildcard
    bool JumpTo(uint16_t Magic);  // Skips instructions untill first occurence of Magic
    void ReverseJumpTo(uint16_t Magic); // Backwards JumpTo (TODO: Merge functions?)
    void SetVariable(const string& Identifier, const Variable& Var); // Sets value of global variable
    void CallScriptSymbol(SymbolType Type);

    void Recover();
    void WriteTrace(std::ostream& Stream);
    void Crash();
    bool NsbAssert(bool expr, const char* fmt);
    void NsbAssert(const char* fmt);
    template<typename T, typename... A> void NsbAssert(const char* fmt, T value, A... args);
    template<typename T, typename... A> bool NsbAssert(bool expr, const char* fmt, T value, A... args);

    Game* pGame;
    NsbContext* pContext;
    NsbContext* pMainContext;
    volatile bool RunInterpreter;
    volatile bool StopInterpreter;

    string HandleName; // Identifier of current Drawable/Playable used by NSB and GL functions
    std::vector<NsbFile*> LoadedScripts; // Scripts considered in symbol lookup
    std::map<string, Variable> Variables; // All local and global variables (TODO: respect scope?)
    std::map<string, ArrayVariable> Arrays; // Same as above, except these are trees
    std::vector<Variable> Params; // Builtin function parameters
    std::vector<ArrayVariable*> ArrayParams; // Tree node parameters
    std::vector<BuiltinFunc> Builtins; // Jump table for builtin functions
    std::list<NsbContext*> Threads;
};

template <> bool NsbInterpreter::GetParam(int32_t Index);
template <> bool NsbInterpreter::NsbAssert(bool expr, const char* fmt, string value);

sf::Texture* LoadTextureFromFile(const string& File, const sf::IntRect& Area);

#endif
