//My lovely structure ^_^
class EntangleSink : public Bufferless<Sink>
{
public:
    EntangleSink(byte ** g_output, size_t * g_size) : output(g_output), out_size(g_size)
    {
        cout << "Constructor called" << endl;
        Clean();
    }

    ~EntangleSink()
    {
        cout << "Destructor called" << endl;
        Clean();
    }

    size_t Put2(const byte *inString, size_t length, int, bool)
    {
        if(!inString || !length)
			return length;
        //Reallocating the array
        *output = (byte*) realloc(*output, *out_size+length);
        //Adding new data
        byte * WhereToJoin = *output + *out_size;
        MemCopy(WhereToJoin, inString, length);
        //Updating the size
        *out_size+=length;
        return 0;
    }

    void Clean()
    {
        cout << "Clean() called" << endl;
        if(*output!=NULL)
        {
            delete[] *output;
            *output = NULL;
        }
        *out_size=0;
    }
private:
    byte ** output;
    size_t * out_size;
};
