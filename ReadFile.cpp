
struct str
{
    const char* str = nullptr;
    int         len =       0;
};

struct Text
{
    size_t num_symbols = 0;
    size_t num_str = 0;
    str* lines = nullptr;
    char* buffer = nullptr;
};

//-----------------------------------------------
//!  calculates file length
//!
//!  @param [in]   in  file
//!
//!  @return lenght of file
//-----------------------------------------------

size_t LenOfFile(FILE* in)
{
    assert(in != nullptr);

    fseek(in, 0, SEEK_END);
    size_t pos = ftell(in);
    fseek(in, 0, SEEK_SET);

    return pos;
}

//-----------------------------------------------
//!  calculating number of lines
//!
//!  @param [in] buffer       pointer on start of text
//!  @param [in] num_symbols  number of symbols in the text
//!
//!  @return number of lines
//-----------------------------------------------

size_t NumLines(const char* buffer, size_t num_symbols)
{
    assert(buffer != nullptr);

    size_t num = 1;
    for (size_t i = 0; i < num_symbols; ++i)
    {
        if (buffer[i] == '\n') 
       	{
       		num++;
        }
    }
    return num;
}

//-----------------------------------------------
//!  reading from file to buffer
//!
//!  @param [in] in           file read from
//!  @param [in] num_symbols  number of symbols in the text
//!  @param [in] buffer        pointer on start of text
//!
//!  @note if you have windows, function will clear text from '\r'
//-----------------------------------------------

size_t ReadFile(FILE* in, size_t num_symbols, char* buffer)
{
    assert(in != nullptr);

    size_t real_num_symbols = fread(buffer, sizeof(char), num_symbols, in);
    #if _WIN32
        buffer[real_num_symbols + 1] = '\0';
    #endif
    return real_num_symbols;
}

//-----------------------------------------------
//!  making lines from the text
//!
//!  @param [in] lines        pointer to struct where kept string and lenght
//!  @param [in] num_symbols  number of symbols in the text
//!  @param [in] buffer        pointer on start of text
//!
//!  @note also calculates lenght of each string
//-----------------------------------------------

void MakeLines(char* buffer, str* lines, size_t num_symbols)
{
    assert(buffer != nullptr);
    assert(lines != nullptr);

    size_t lastI    = 0;
    size_t num_line = 0;

    lines[num_line++].str = buffer;

    for (size_t i = 0; i < num_symbols; ++i)
    {
        if (buffer[i] == '\n')
        {
            buffer[i] = '\0';
            lines[num_line - 1].len = i - lastI;
            lastI = i + 1;
            lines[num_line++].str = &buffer[i+1];
        }
    }
    lines[num_line - 1].len = num_symbols - lastI;
}

//-----------------------------------------------
//!  reading file, making lines, calculating number of
//!  lines and symbols
//!
//!  @param [in] text  pointer to struct text
//!  @param [in] in    file read from
//!
//!  @note text includes num_symbols, num_str,
//!             pointer start and pointer lines
//-----------------------------------------------

void ReadTextAndMakeLines(Text* text, FILE* in)
{
    text->num_symbols = LenOfFile(in);
    text->buffer       = (char*)calloc(text->num_symbols + 1, sizeof(char));
    text->num_symbols = ReadFile(in, text->num_symbols, text->buffer);
    
    text->num_str     = NumLines(text->buffer, text->num_symbols);
    text->lines       = (str*)calloc(text->num_str + 1, sizeof(str));
    MakeLines(text->buffer, text->lines, text->num_symbols);
}

//-----------------------------------------------
//!  free all dynamic memory
//!
//!  @param [in] text  pointer to struct text
//-----------------------------------------------

void FreeAll(Text* text)
{
    assert(text->buffer);
    assert(text->lines);

    free(text->buffer);
         text->buffer = nullptr;
    free(text->lines);
         text->lines = nullptr;
}
