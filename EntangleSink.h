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
        //Reserving new array
        byte * NewArray = new byte[*out_size+length];
        memset(NewArray, 0, *out_size+length); //!!!
        if(*output!=NULL)
        {
            //Copying there old data
            memcpy(NewArray, *output, *out_size);
            //Deleting old array
            delete[] *output;
        }
        //Adding new data
        byte * WhereToJoin = NewArray + *out_size;
        memcpy(WhereToJoin, inString, length);
        //Updating the data address
        *output = NewArray;
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
