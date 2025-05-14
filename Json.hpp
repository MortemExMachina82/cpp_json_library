#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>


#ifndef JSON
#define JSON

#ifndef IsWhiteSpace
    #define IsWhiteSpace(C) (C == ' ' || C == '\t' || C == '\n' || C == '\r')
#endif
#ifndef IsEndOfItem
    #define IsEndOfItem(C) (C == ',' || C == '}' || C == ']')
#endif

#ifndef IsValidNumber
    #define IsValidNumber(C) (C == '.' || C == '-' || (C >= 48 && C <= 57))
#endif

#ifndef JSON_INDENT
    #define JSON_INDENT 2
#endif


enum JsonItemType{
    Json_Null,
    Json_String,
    Json_Object,
    Json_Double,
    Json_Long,
    Json_Array,
    Json_Bool
};

struct JsonItem;

//Json object type
//stores JsonItems
class JsonObject {
    std::vector<JsonItem> Items;
public:
    inline JsonItem Get(std::string Name);
    inline size_t GetPosition(std::string Name);
    inline void Add(JsonItem Item);
    inline void Replace(JsonItem Item, std::string Name);
    //returns chars processd
    inline size_t FromString(const std::string String);
    inline std::string ToString(int Indent);

    inline size_t size() {
        return this->Items.size();
    }
    inline std::vector<JsonItem>::iterator begin();
    inline std::vector<JsonItem>::iterator end();
    inline operator std::string() {
        return this->ToString(0);
    }
    inline JsonItem operator [] (size_t Index);
    inline JsonObject& operator+= (const JsonItem& Item);
};

//Json array type
//stores JsonItems
class JsonArray {
    std::vector<JsonItem> Items;
public:
    inline JsonItem Get(size_t index);
    inline void Add(JsonItem Item);
    inline void Replace(JsonItem Item, size_t Index);
    //returns chars processd
    inline size_t FromString(const std::string String);
    inline std::string ToString(int Indent);

    inline size_t size() {
        return this->Items.size();
    }
    inline std::vector<JsonItem>::iterator begin();
    inline std::vector<JsonItem>::iterator end();
    inline operator std::string() {
        return this->ToString(0);
    }
    inline JsonItem& operator [] (size_t I);
    inline JsonArray& operator+= (const JsonItem& Item);
};

//Json name value pair
struct JsonItem {
    std::string Name = "";
    JsonItemType Type = Json_Null;

    std::string Value_String = "";
    JsonObject Value_Object;
    JsonArray Value_Array;
    double Value_Double = 0;
    long long Value_Long = 0;
    bool Value_Bool = false;
    inline JsonItem() {
        this->Name = "";
        this->Type = Json_Null;
        this->Value_Bool = false;
        this->Value_Double = 0;
        this->Value_Long = 0;
    }
    inline JsonItem(std::string Name, std::string Value) {
        this->Name = Name;
        this->Type = Json_String;
        this->Value_String = Value;
    };
    inline JsonItem(std::string Name, JsonObject Value) {
        this->Name = Name;
        this->Type = Json_Object;
        this->Value_Object = Value;
    };
    inline JsonItem(std::string Name, JsonArray Value) {
        this->Name = Name;
        this->Type = Json_Array;
        this->Value_Array = Value;
    };
    inline JsonItem(std::string Name, float Value) {
        this->Name = Name;
        this->Type = Json_Double;
        this->Value_Double = Value;
        this->Value_Long = long long(Value);
    };
    inline JsonItem(std::string Name, double Value) {
        this->Name = Name;
        this->Type = Json_Double;
        this->Value_Double = Value;
        this->Value_Long = long long(Value);
    };
    inline JsonItem(std::string Name, int Value) {
        this->Name = Name;
        this->Type = Json_Long;
        this->Value_Long = Value;
        this->Value_Double = double(Value);
    };
    inline JsonItem(std::string Name, long Value) {
        this->Name = Name;
        this->Type = Json_Long;
        this->Value_Long = Value;
        this->Value_Double = double(Value);
    };
    inline JsonItem(std::string Name, long long Value) {
        this->Name = Name;
        this->Type = Json_Long;
        this->Value_Long = Value;
        this->Value_Double = double(Value);
    };
    inline JsonItem(std::string Name, bool Value) {
        this->Name = Name;
        this->Type = Json_Bool;
        this->Value_Bool = Value;
        this->Value_Long = long long(Value);
        this->Value_Double = double(Value);
    };
    inline JsonItem(std::string Name) {
        this->Name = Name;
        this->Type = Json_Null;
        this->Value_Bool = false;
        this->Value_Double = 0;
        this->Value_Long = 0;
    }

    //returns number of chars processed - 1
    //throws exception when encountering syntax errors
    inline size_t ParseString(const std::string Input, std::string& Out) {
        bool FoundFirstQuote = false;
        size_t I = 0;
        for (; I < Input.size(); I++) {
            char C = Input[I];
            if (C == '"') {
                if (FoundFirstQuote) {
                    return I;
                }
                FoundFirstQuote = true;
                continue;
            }
            if (!FoundFirstQuote) {
                continue;
            }

            if (C == '\\') {
                I++;
                if (I >= Input.size()) {
                    throw std::exception("Error Parsing Json: Ran Out Of Input");
                }
                C = Input[I];
                switch (C) {
                case '"':
                    C = '"';
                    break;
                case '\\':
                    C = '\\';
                    break;
                case '/':
                    C = '/';
                    break;
                case 'b':
                    C = '\b';
                    break;
                case 'f':
                    C = '\f';
                    break;
                case 'n':
                    C = '\n';
                    break;
                case 'r':
                    C = '\r';
                    break;
                case 't':
                    C = '\t';
                    break;
                case 'u':
                    //FIXME
                    break;
                }
                Out += C;
                continue;
            }
            Out += C;
        }
    }
    //returns number of chars processed - 1
    //throws exception when encountering syntax errors
    inline size_t FromString(const std::string String, bool InArray) {
        bool ParsedString = InArray;
        size_t I = 0;
        for (; I < String.size(); I++) {
            char C = String[I];
            if (IsWhiteSpace(C)) {
                continue;
            }
            if (!ParsedString) {
                if (C == '"') {
                    I += ParseString(String.substr(I), Name);
                    for (; I < String.size(); I++) {
                        if (String[I] == ':') {
                            break;
                        }
                    }
                    ParsedString = true;
                }
                continue;
            }
            if (C == '"') {
                Type = Json_String;
                I += ParseString(String.substr(I), Value_String);
                return I;
            }
            if (C == '{') {
                Type = Json_Object;
                I += Value_Object.FromString(String.substr(I));
                return I;
            }
            if (C == '[') {
                Type = Json_Array;
                I += Value_Array.FromString(String.substr(I));
                return I;
            }
            if (C == 't') {
                if (String.substr(I, 4) == "true") {
                    Type = Json_Bool;
                    Value_Bool = true;
                    I += 4;
                    return I;
                }
            }
            if(C == 'f'){
                if (String.substr(I, 5) == "false") {
                    Type = Json_Bool;
                    Value_Bool = false;
                    I += 5;
                    return I;
                }
            }
            if (C == 'n') {
                if (String.substr(I, 4) == "null") {
                    Type = Json_Null;
                    I += 4;
                    return I;
                }
            }
            bool IsDouble = false;
            size_t NChars = 0;
            for (size_t I2 = I; I2 < String.size(); I2++) {
                char C2 = String[I2];
                if (IsWhiteSpace(C2)) {
                    continue;
                }
                if (IsEndOfItem(C2)) {
                    break;
                }
                if (!IsValidNumber(C2)) {
                    throw std::exception(("Error Parsing Json: Encontered Invalid Charector At "+std::to_string(I2)).c_str());
                }
                if (C2 == '.') {
                    IsDouble = true;
                }
                NChars++;
            }
            if (IsDouble) {
                Type = Json_Double;
                Value_Double = std::stod(String.substr(I), &NChars);
                Value_Long = Value_Double;
                I += (NChars-1);
                return I;
            }
            else {
                Type = Json_Long;
                Value_Long = std::stoll(String.substr(I), &NChars);
                Value_Double = Value_Long;
                I += (NChars-1);
                return I;
            }
        }
        return I;
    }
    
    inline std::string ToString(int Indent) {
        std::string Out;
        for (int I = 0; I < Indent; I++){Out += ' ';}
        if (Name != "") {
            Out += '\"' + Name + "\":";
        }
        if (Type == Json_Null) {
            Out += "null";
            return Out;
        }
        if (Type == Json_String) {
            Out += '"';
            for (char C : Value_String) {
                switch (C) {
                case '\\':
                    Out += "\\\\";
                    break;
                case '"':
                    Out += "\\\"";
                    break;
                case '/':
                    Out += "\\/";
                    break;
                case '\b':
                    Out += "\\\b";
                    break;
                case '\f':
                    Out += "\\\f";
                    break;
                case '\n':
                    Out += "\\\n";
                    break;
                case '\r':
                    Out += "\\\r";
                    break;
                case '\t':
                    Out += "\\\t";
                    break;
                default:
                    Out += C;
                    break;
                }
            }
            Out += '"';
            return Out;
        }
        if (Type == Json_Object) {
            Out += Value_Object.ToString(Indent);
            return Out;
        }
        if (Type == Json_Array) {
            Out += Value_Array.ToString(Indent);
            return Out;
        }
        if (Type == Json_Bool) {
            if (Value_Bool) {
                Out += "true";
            }
            else {
                Out += "false";
            }
            return Out;
        }
        if (Type == Json_Double) {
            char D[64];
            int size = snprintf(D, 64, "%f", Value_Double);
            Out += D;
            return Out;
        }
        if (Type == Json_Long) {
            char D[64];
            int size = snprintf(D, 64, "%lld", Value_Long);
            Out += D;
            return Out;
        }
    }

    operator bool() {
        return this->Value_Bool;
    }
    operator std::string() {
        return this->Value_String;
    }
    operator const char* () {
        return this->Value_String.c_str();
    }
    operator float() {
        return float(this->Value_Double);
    }
    operator double() {
        return this->Value_Double;
    }
    operator int() {
        return this->Value_Long;
    }
    operator long() {
        return this->Value_Long;
    }
    operator long long() {
        return this->Value_Long;
    }
    operator size_t() {
        return size_t(this->Value_Long);
    }
    operator JsonObject() {
        return this->Value_Object;
    }
    operator JsonArray() {
        return this->Value_Array;
    }
};

inline std::vector<JsonItem>::iterator JsonObject::begin() {
    return Items.begin();
}
inline std::vector<JsonItem>::iterator JsonObject::end() {
    return Items.end();
}
inline JsonItem JsonObject::Get(std::string Name) {
    for (unsigned int X = 0; X < Items.size(); X++) {
        JsonItem Item = Items.at(X);
        if (Item.Name == Name) {
            return Item;
        }
    }
    JsonItem NullItem;
    NullItem.Type = Json_Null;
    NullItem.Value_Bool = false;
    NullItem.Value_Double = 0;
    NullItem.Value_Long = 0;
    NullItem.Value_String = "Item Does Not Exist";
    return NullItem;
}
inline size_t JsonObject::GetPosition(std::string Name) {
    for (size_t X = 0; X < Items.size(); X++) {
        JsonItem Item = Items.at(X);
        if (Item.Name == Name) {
            return X;
        }
    }
    return 0;
}
inline void JsonObject::Add(JsonItem Item) {
    Items.push_back(Item);
}
inline void JsonObject::Replace(JsonItem Item, std::string Name) {
    size_t Index = GetPosition(Name);
    if (Index >= Items.size()) {
        return;
    }
    Items[Index] = Item;
}
//returns number of chars processed - 1 eg:
//{"eg":123}
//returns 9
//throws exception when encountering syntax errors
inline size_t JsonObject::FromString(const std::string String) {
    bool FoundFirstBracket = false;
    for (size_t I = 0; I < String.size(); I++) {
        char C = String[I];
        if (IsWhiteSpace(C)) {
            continue;
        }
        if (C == '{') {
            FoundFirstBracket = true;
            continue;
        }
        if (C == '[') {
            JsonItem Item;
            I += Item.Value_Array.FromString(String.substr(I));
            Items.push_back(Item);
            return I;
        }
        if (!FoundFirstBracket) {
            continue;
        }
        if (C == '"') {
            JsonItem Item;
            I += Item.FromString(String.substr(I), false);
            Items.push_back(Item);
            continue;
        }
        if (C == '}') {
            return I;
        }
    }
}
//if object contains only 1 unnamed item of type JsonArray
//object will be treated like JsonArray
inline std::string JsonObject::ToString(int Indent) {
    std::string Out;

    if (Items.size() == 1) {
        if (Items[0].Name == "") {
            if (Items[0].Type == Json_Array) {
                Out += Items[0].ToString(Indent);
                return Out;
            }
        }
    }

    Out += "{\n";
    for (size_t I = 0; I < Items.size(); I++) {
        Out += Items[I].ToString(Indent + JSON_INDENT);
        if (I < Items.size() - 1) {
            Out += ",";
        }
        Out += "\n";
    }
    for (int I = 0; I < Indent; I++) { Out += ' '; }
    Out += "}";
    return Out;
}
inline JsonItem JsonObject::operator [] (size_t Index) {
    if (Index < size()) {
        return this->Items[Index];
    }
    return JsonItem("NotFound", 0);
}
inline JsonObject& JsonObject::operator+= (const JsonItem& Item) {
    Add(Item);
    return *this;
}


inline std::vector<JsonItem>::iterator JsonArray::begin() {
    return Items.begin();
}
inline std::vector<JsonItem>::iterator JsonArray::end() {
    return Items.end();
}
inline JsonItem JsonArray::Get(size_t index) {
    return Items.at(index);
}
inline void JsonArray::Add(JsonItem Item) {
    Items.push_back(Item);
}
inline void JsonArray::Replace(JsonItem Item, size_t Index) {
    if (Index >= Items.size()) {
        return;
    }
    Items[Index] = Item;
}
inline JsonItem& JsonArray::operator [] (size_t I) {
    return this->Items[I];
}
//returns number of chars processed - 1 eg:
//["eg",123]
//returns 9
//throws exception when encountering syntax errors
inline size_t JsonArray::FromString(const std::string String) {
    bool FoundFirstBracket = false;
    for (size_t I = 0; I < String.size(); I++) {
        char C = String[I];
        if (IsWhiteSpace(C)) {
            continue;
        }
        if (C == '[') {
            FoundFirstBracket = true;
            continue;
        }
        if (!FoundFirstBracket) {
            continue;
        }
        if (C == ']') {
            return I;
        }
        if (C == ',') {
            continue;
        }
        JsonItem Item;
        I += Item.FromString(String.substr(I), true);
        Items.push_back(Item);
    }
}
inline std::string JsonArray::ToString(int Indent) {
    std::string Out;
    Out += "[\n";
    for (size_t I = 0; I < Items.size(); I++) {
        Out += Items[I].ToString(Indent + JSON_INDENT);
        if (I < Items.size() - 1) {
            Out += ",";
        }
        Out += "\n";
    }
    for (int I = 0; I < Indent; I++) { Out += ' '; }
    Out += "]";
    return Out;
}
inline JsonArray& JsonArray::operator+= (const JsonItem& Item) {
    Add(Item);
    return *this;
}


//Loads Json file
//throws exception when encountering syntax errors
inline void LoadJson(std::string Path, JsonObject& Object) {
    char* Contents = nullptr;
    size_t ContentsLength;
    std::ifstream file;
    file.open(Path, std::ios_base::in | std::ios_base::binary);
    if (file.fail() || !file.is_open()) {
        file.close();
        throw std::exception(("Error Unable To Open File " + Path + "\n").c_str());
    }
    file.seekg(0, std::ios_base::end);
    ContentsLength = file.tellg();
    Contents = new char[ContentsLength];
    file.seekg(0, std::ios_base::beg);
    file.read(&Contents[0], ContentsLength);
    file.close();

    JsonObject O;
    O.FromString(Contents);
    delete[] Contents;
    Object = O;
}
//Loads Json file containing only array
//throws exception when encountering syntax errors
inline void LoadJson(std::string Path, JsonArray& Array) {
    char* Contents = nullptr;
    size_t ContentsLength;
    std::ifstream file;
    file.open(Path, std::ios_base::in | std::ios_base::binary);
    if (file.fail() || !file.is_open()) {
        file.close();
        throw std::exception(("Error Unable To Open File " + Path + "\n").c_str());
    }
    file.seekg(0, std::ios_base::end);
    ContentsLength = file.tellg();
    Contents = new char[ContentsLength];
    file.seekg(0, std::ios_base::beg);
    file.read(&Contents[0], ContentsLength);
    file.close();

    JsonArray A;
    A.FromString(Contents);
    delete[] Contents;
    Array = A;
}
//writes JsonObject to file
//if object contains only 1 unnamed item of type JsonArray
//object will be treated like JsonArray
inline bool WriteJson(std::string Path, JsonObject Object) {
    std::string Data = Object.ToString(0);

    std::filesystem::path PPath = std::filesystem::path(Path).parent_path();
    if (!std::filesystem::exists(PPath)) {
        std::filesystem::create_directories(PPath);
    }

    std::ofstream File;
    File.open(Path.c_str(), std::ios_base::out | std::ios_base::binary);
    if (!File.is_open()) {
        File.close();
        printf("Could Not Open File For Writing\n%s\n", Path.c_str());
        return false;
    }

    File.write(Data.c_str(), Data.size());
    return true;
}


#endif //ifndef JSON