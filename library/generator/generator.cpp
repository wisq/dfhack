#include <iostream>
#include <algorithm>
#include <tinyxml.h>
#include <tinystr.h>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <dfhack/DFIntegers.h>
using namespace std;

class indentr
{
public:
    friend std::basic_ostream<char>& operator <<(std::basic_ostream<char>&, const indentr &);
    indentr(int ns = 0, int size = 4)
    {
        numspaces = ns;
        step = size;
    }
    void indent ()
    {
        numspaces += step;
        if (numspaces < 0) numspaces = 0;
    }
    void unindent ()
    {
        numspaces -= step;
        if (numspaces < 0) numspaces = 0;
    }
    int get ()
    {
        return numspaces;
    }
    void set (int ns)
    {
        numspaces = ns;
        if (numspaces < 0) numspaces = 0;
    }
    private:
    int step;
    int numspaces;
};
std::basic_ostream<char>& operator<< (std::basic_ostream<char>& os, const indentr & idtr)
{
    for(int i = 0; i < idtr.numspaces ;i++)
    {
        os << ' ';
    }
    return os;
}

class OffsetGroup
{
protected:
    set <string> addresses;
    set <string> hexvals;
    set <string> offsets;
    set <string> strings;
    set <OffsetGroup *> groups;
    std::string name;
    OffsetGroup * parent;
public:
    OffsetGroup(const std::string & _name, OffsetGroup * parent = 0);
    ~OffsetGroup();

    virtual string toDeclaration(uint32_t indent);
    virtual string toReader();
    void createOffset (const std::string & key);
    void createAddress (const std::string & key);
    void createHexValue (const std::string & key);
    void createString (const std::string & key);
    OffsetGroup * createGroup ( const std::string & name );

    std::string getName();
    std::string getFullName();
    OffsetGroup * getParent();
};

class Base : public OffsetGroup
{
public:
    Base(const string & _name);
    ~Base();
    virtual string toDeclaration(uint32_t indent);
    virtual string toReader();
};

void OffsetGroup::createOffset(const string & key)
{
    offsets.insert(key);
}

void OffsetGroup::createAddress(const string & key)
{
    addresses.insert(key);
}

void OffsetGroup::createHexValue(const string & key)
{
    hexvals.insert(key);
}

void OffsetGroup::createString(const string & key)
{
    strings.insert(key);
}

OffsetGroup * OffsetGroup::createGroup(const std::string &name)
{
    std::set< OffsetGroup* >::iterator iter = groups.begin();
    while (iter != groups.end())
    {
        if((*iter)->getName() == name)
            return (*iter);
        iter ++;
    }
    OffsetGroup * ret = new OffsetGroup(name, this);
    groups.insert(ret);
    return ret;
}

Base::Base(const std::string& _name): OffsetGroup(_name)
{
}

string Base::toDeclaration(uint32_t indent)
{
    ostringstream os;
    indentr i(indent);
    os << i << "struct " << name << "_offsets" << endl << i << "{" << endl;
    i.indent();
    set< string >::iterator iter;
    for( iter = addresses.begin(); iter != addresses.end(); iter++)
    {
        os << i << "Address " << *iter << ";" << endl;
    }
    for(iter = offsets.begin(); iter != offsets.end(); iter++)
    {
        os << i << "Offset " << *iter << ";" << endl;
    }
    for(iter = hexvals.begin(); iter != hexvals.end(); iter++)
    {
        os << i << "HexValue " << *iter << ";" << endl;
    }
    for(iter = strings.begin(); iter != strings.end(); iter++)
    {
        os << i << "DFString " << *iter << ";" << endl;
    }
    for(std::set< OffsetGroup* >::iterator iter4 = groups.begin(); iter4 != groups.end(); iter4++)
    {
        os << (*iter4)->toDeclaration(i.get());
    }
    i.unindent();
    os << i << "};" << endl;
    return os.str();
}

string Base::toReader()
{
    return OffsetGroup::toReader();
}

Base::~Base()
{
    this->OffsetGroup::~OffsetGroup();
}

string OffsetGroup::toDeclaration(uint32_t indent)
{
    ostringstream os;
    indentr i(indent);
    os << i << "struct grp_" << name << endl << i << "{" << endl;
    i.indent();
    set< string >::iterator iter;
    for( iter = addresses.begin(); iter != addresses.end(); iter++)
    {
        os << i << "Address " << *iter << ";" << endl;
    }
    for(iter = offsets.begin(); iter != offsets.end(); iter++)
    {
        os << i << "Offset " << *iter << ";" << endl;
    }
    for(iter = hexvals.begin(); iter != hexvals.end(); iter++)
    {
        os << i << "HexValue " << *iter << ";" << endl;
    }
    for(iter = strings.begin(); iter != strings.end(); iter++)
    {
        os << i << "DFString " << *iter << ";" << endl;
    }
    for(std::set< OffsetGroup* >::iterator iter4 = groups.begin(); iter4 != groups.end(); iter4++)
    {
        os << (*iter4)->toDeclaration(i.get());
    }
    i.unindent();
    os << i << "} " << name << ";" << endl;
    return os.str();
}

string OffsetGroup::toReader()
{
    return "bleh\n";
}

OffsetGroup::OffsetGroup(const std::string & name, OffsetGroup * parent)
{
    this->name = name;
    this->parent = parent;
}

OffsetGroup::~OffsetGroup()
{
    for( std::set< OffsetGroup* >::iterator it = groups.begin();it != groups.end();it++)
    {
        delete (*it);
    }
    groups.clear();
}

std::string OffsetGroup::getName()
{
    return name;
}

OffsetGroup * OffsetGroup::getParent()
{
    return parent;
}

std::string OffsetGroup::getFullName()
{
    string temp, accum;
    OffsetGroup * curr = this;
    while(curr)
    {
        temp = curr->getName() + string("/") + accum;
        accum = temp;
        curr = curr->getParent();
    }
    return accum;
}


void ParseOffsets(TiXmlElement * parent, OffsetGroup* target)
{
    // we parse the groups iteratively instead of recursively
    // breadcrubs acts like a makeshift stack
    // first pair entry stores the current element of that level
    // second pair entry the group object from OffsetGroup
    typedef pair < TiXmlElement *, OffsetGroup * > groupPair;
    vector< groupPair > breadcrumbs;
    {
        TiXmlElement* pEntry;
        // we get the <Offsets>, look at the children
        pEntry = parent->FirstChildElement();
        if(!pEntry)
            return;

        breadcrumbs.push_back(groupPair(pEntry,target));
    }

    // work variables
    OffsetGroup * currentGroup = 0;
    TiXmlElement * currentElem = 0;
    //cerr << "<Offsets>"<< endl;
    while(1)
    {
        // get current work variables
        currentElem = breadcrumbs.back().first;
        currentGroup = breadcrumbs.back().second;

        // we reached the end of the current group?
        if(!currentElem)
        {
            // go one level up
            breadcrumbs.pop_back();
            // exit if no more work
            if(breadcrumbs.empty())
            {
                break;
            }
            else
            {
                //cerr << "</group>" << endl;
                continue;
            }
        }

        if(!currentGroup)
        {
            groupPair & gp = breadcrumbs.back();
            gp.first = gp.first->NextSiblingElement();
            continue;
        }

        // skip non-elements
        if (currentElem->Type() != TiXmlNode::ELEMENT)
        {
            groupPair & gp = breadcrumbs.back();
            gp.first = gp.first->NextSiblingElement();
            continue;
        }

        // we have a valid current element and current group
        // get properties
        string type = currentElem->Value();
        std::transform(type.begin(), type.end(), type.begin(), ::tolower);
        const char *cstr_name = currentElem->Attribute("name");
        if(!cstr_name)
        {
            groupPair & gp = breadcrumbs.back();
            gp.first = gp.first->NextSiblingElement();
            cerr << "Entry has no name! line: " << currentElem->Row() <<endl;
            continue;
        }

        // evaluate elements
        if(type == "group")
        {
            // create group
            OffsetGroup * og;
            og = currentGroup->createGroup(cstr_name);
            // advance this level to the next element
            groupPair & gp = breadcrumbs.back();
            gp.first = currentElem->NextSiblingElement();
            // add a new level that will be processed next
            breadcrumbs.push_back(groupPair(currentElem->FirstChildElement(), og));
            continue;
        }
        else if(type == "address")
        {
            currentGroup->createAddress(cstr_name);
        }
        else if(type == "offset")
        {
            currentGroup->createOffset(cstr_name);
        }
        else if(type == "string")
        {
            currentGroup->createString(cstr_name);
        }
        else if(type == "hexvalue")
        {
            currentGroup->createHexValue(cstr_name);
        }

        // advance to next element
        groupPair & gp = breadcrumbs.back();
        gp.first = currentElem->NextSiblingElement();
        continue;
    }
    //cerr << "</Offsets>"<< endl;
}


void ParseBase (TiXmlElement* entry, OffsetGroup* mem)
{
    TiXmlElement* pElement;
    TiXmlElement* pElement2nd;
    const char *cstr_version = entry->Attribute("name");

    if (!cstr_version)
    {
        cerr << "Base has no name at row " << entry->Row() << endl;
        return;
    }

    // process additional entries
    pElement = entry->FirstChildElement()->ToElement();
    for(;pElement;pElement=pElement->NextSiblingElement())
    {
        // only elements get processed
        const char *cstr_type = pElement->Value();
        std::string type = cstr_type;
        if(type == "VTable")
        {
            // do nothing
            // parsevtables
            continue;
        }
        else if(type == "Offsets")
        {
            // we don't care about the descriptions here, do nothing
            ParseOffsets(pElement, mem);
            continue;
        }
        else if (type == "Professions")
        {
            pElement2nd = pElement->FirstChildElement("Profession");
            for(;pElement2nd;pElement2nd=pElement2nd->NextSiblingElement("Profession"))
            {
                const char * id = pElement2nd->Attribute("id");
                const char * name = pElement2nd->Attribute("name");
                // FIXME: missing some attributes here
                if(id && name)
                {
                    continue;
                }
                else
                {
                    cerr << "Underspecified Profession tag at line " << pElement2nd->Row() << endl;
                    continue;
                }
            }
        }
        else if (type == "Jobs")
        {
            pElement2nd = pElement->FirstChildElement("Job");
            for(;pElement2nd;pElement2nd=pElement2nd->NextSiblingElement("Job"))
            {
                const char * id = pElement2nd->Attribute("id");
                const char * name = pElement2nd->Attribute("name");
                if(id && name)
                {
                    continue;
                }
                else
                {
                    cerr << "Underspecified Job tag at line " << pElement2nd->Row() << endl;
                    continue;
                }
            }
        }
        else if (type == "Skills")
        {
            pElement2nd = pElement->FirstChildElement("Skill");
            for(;pElement2nd;pElement2nd=pElement2nd->NextSiblingElement("Skill"))
            {
                const char * id = pElement2nd->Attribute("id");
                const char * name = pElement2nd->Attribute("name");
                if(id && name)
                {
                    continue;
                }
                else
                {
                    cerr << "Underspecified Skill tag at line " << pElement2nd->Row() << endl;
                    continue;
                }
            }
        }
        else if (type == "Traits")
        {
            pElement2nd = pElement->FirstChildElement("Trait");
            for(;pElement2nd;pElement2nd=pElement2nd->NextSiblingElement("Trait"))
            {
                const char * id = pElement2nd->Attribute("id");
                const char * name = pElement2nd->Attribute("name");
                const char * lvl0 = pElement2nd->Attribute("level_0");
                const char * lvl1 = pElement2nd->Attribute("level_1");
                const char * lvl2 = pElement2nd->Attribute("level_2");
                const char * lvl3 = pElement2nd->Attribute("level_3");
                const char * lvl4 = pElement2nd->Attribute("level_4");
                const char * lvl5 = pElement2nd->Attribute("level_5");
                if(id && name && lvl0 && lvl1 && lvl2 && lvl3 && lvl4 && lvl5)
                {
                    continue;
                }
                else
                {
                    cerr << "Underspecified Trait tag at line " << pElement2nd->Row() << endl;
                    continue;
                }
            }
        }
        else if (type == "Labors")
        {
            pElement2nd = pElement->FirstChildElement("Labor");
            for(;pElement2nd;pElement2nd=pElement2nd->NextSiblingElement("Labor"))
            {
                const char * id = pElement2nd->Attribute("id");
                const char * name = pElement2nd->Attribute("name");
                if(id && name)
                {
                    continue;
                }
                else
                {
                    cerr << "Underspecified Labor tag at line " << pElement2nd->Row() << endl;
                    continue;
                }
            }
        }
        else if (type == "Levels")
        {
            pElement2nd = pElement->FirstChildElement("Level");
            for(;pElement2nd;pElement2nd=pElement2nd->NextSiblingElement("Level"))
            {
                const char * id = pElement2nd->Attribute("id");
                const char * name = pElement2nd->Attribute("name");
                const char * nextlvl = pElement2nd->Attribute("xpNxtLvl");
                if(id && name && nextlvl)
                {
                    continue;
                }
                else
                {
                    cerr << "Underspecified Level tag at line " << pElement2nd->Row() << endl;
                    continue;
                }
            }
        }
        else if (type == "Moods")
        {
            pElement2nd = pElement->FirstChildElement("Mood");
            for(;pElement2nd;pElement2nd=pElement2nd->NextSiblingElement("Mood"))
            {
                const char * id = pElement2nd->Attribute("id");
                const char * name = pElement2nd->Attribute("name");
                if(id && name)
                {
                    continue;
                }
                else
                {
                    cerr << "Underspecified Mood tag at line " << pElement2nd->Row() << endl;
                    continue;
                }
            }
        }
        else
        {
            cerr << "Unknown tag " << type << " at line " << pElement->Row() << endl;
            continue;
        }
    } // for
} // method

// load the XML file with offsets
bool loadFile(string path_to_xml, vector <Base *> & bases)
{
    TiXmlDocument doc( path_to_xml.c_str() );
    //bool loadOkay = doc.LoadFile();
    if (!doc.LoadFile())
    {
        cerr << "Can't load file: " << path_to_xml << ", " << doc.ErrorDesc() << ", line " << doc.ErrorRow() << endl;
        return false;
    }
    TiXmlHandle hDoc(&doc);
    TiXmlElement* pElem;
    TiXmlHandle hRoot(0);

    // block: name
    {
        pElem=hDoc.FirstChildElement().Element();
        // should always have a valid root but handle gracefully if it does
        if (!pElem)
        {
            cerr << "XML file has no root element" << endl;
            return false;
        }
        string m_name=pElem->Value();
        if(m_name != "DFHack")
        {
            cerr << "Root element isn't DFHack'" << endl;
            return false;
        }
        // save this for later
        hRoot=TiXmlHandle(pElem);
    }
    // transform elements
    {
        // trash existing list
        for(uint32_t i = 0; i < bases.size(); i++)
        {
            delete bases[i];
        }
        bases.clear();

        // For each base version, get offsets structure
        TiXmlElement* pMemInfo=hRoot.FirstChild( "Base" ).Element();
        map <string ,TiXmlElement *> map_pNamedEntries;
        vector <string> v_sEntries;
        for( ; pMemInfo; pMemInfo=pMemInfo->NextSiblingElement("Base"))
        {
            const char *name = pMemInfo->Attribute("name");
            if(name)
            {
                string str_name = name;
                Base *base = new Base(name);
                ParseBase( pMemInfo , base );
                bases.push_back(base);
            }
        }
    }
    return true;
}



int main ( int argc, char** argv )
{
    if(argc < 2)
    {
        cout << "A file is required!" << ::std::endl;
        return EXIT_FAILURE;
    }
    vector <Base *> bases;
    if(loadFile(argv[1],bases))
    {
        for(int i = 0; i < bases.size(); i++)
        {
            cout << bases[i]->toDeclaration(0);
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}