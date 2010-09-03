#include <iostream>
#include <algorithm>
#include <depends/tinyxml/tinyxml.h>
#include <depends/tinyxml/tinystr.h>

using namespace std;

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
    OffsetGroup();
    OffsetGroup(const std::string & _name, OffsetGroup * parent = 0);
    ~OffsetGroup();

//    void copy(const OffsetGroup * old); // recursive
//    void RebaseAddresses( int32_t offset ); // recursive

    void createOffset (const std::string & key);
    void createAddress (const std::string & key);
    void createHexValue (const std::string & key);
    void createString (const std::string & key);
    OffsetGroup * createGroup ( const std::string & name );

    std::string getName();
    std::string getFullName();
    OffsetGroup * getParent();
};

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

        OffsetGroup * currentGroup = target;
        breadcrumbs.push_back(groupPair(pEntry,currentGroup));
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
            // ERROR, missing attribute
        }

        // evaluate elements
        const char *cstr_value = currentElem->Attribute("value");
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
            ParseOffsets(pElement, mem, true);
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
bool loadFile(string path_to_xml, vector <OffsetGroup *> & bases)
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
    VersionInfo *mem;

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
                OffsetGroup *base = new OffsetGroup();
                ParseBase( pMemInfo , mem );
                bases.push_back(base);
            }
        }
    }
    return true;
}



int main ( int argc, char** argv )
{
    cout << "lolz" << endl;
}