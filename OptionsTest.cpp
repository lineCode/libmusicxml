#include <iostream>
#include <sstream>

#include <vector>
#include <list>
#include <map>

#include <algorithm>

#include <iomanip>      // setw, set::precision, ...


using namespace std;

#define EXP

//______________________________________________________________________________
class EXP msrOptionsElement
{
  public:

    // creation from MusicXML
    // ------------------------------------------------------

  protected:

    msrOptionsElement (
      string optionsElementShortName,
      string optionsElementLongName,
      string optionsElementDescription);

    virtual ~msrOptionsElement();

  public:

    // set and get
    // ------------------------------------------------------

    string                getOptionsElementShortName () const
                              { return fOptionsElementShortName; }

    string                getOptionsElementLongName () const
                              { return fOptionsElementLongName; }

    string                getOptionsElementDescription () const
                              { return fOptionsElementDescription; }

    // services
    // ------------------------------------------------------

    string                operator() () const
                              { return fOptionsElementDescription; }
  
    // print
    // ------------------------------------------------------

    virtual void          print (ostream& os) const;
    
  protected:
     
    string                fOptionsElementShortName;
    string                fOptionsElementLongName;
    string                fOptionsElementDescription;
};
//typedef SMARTP<msrOptionsElement> S_msrOptionsElement;
typedef msrOptionsElement* S_msrOptionsElement;
EXP ostream& operator<< (ostream& os, const S_msrOptionsElement& elt);


msrOptionsElement::msrOptionsElement (
  string optionsElementShortName,
  string optionsElementLongName,
  string optionsElementDescription)
{
  fOptionsElementShortName   = optionsElementShortName;  
  fOptionsElementLongName    = optionsElementLongName;  
  fOptionsElementDescription = optionsElementDescription;  
}

msrOptionsElement::~msrOptionsElement()
{}

ostream& operator<< (ostream& os, const S_msrOptionsElement& elt)
{
  elt->print (os);
  return os;
}

void msrOptionsElement::print (ostream& os) const
{
  os << "??? msrOptionsElement ???" << endl;
}

//______________________________________________________________________________
class EXP msrOptionsItem : public msrOptionsElement
{
  public:
  
    msrOptionsItem (
      string optionsItemShortName,
      string optionsItemLongName,
      string optionsItemDescription)
      : msrOptionsElement (
          optionsItemShortName,
          optionsItemLongName,
          optionsItemDescription)
      {
        fOptionsItemHasBeenSelected = false;
      }
    
    void                  setOptionsItemHasBeenSelected ()
                              { fOptionsItemHasBeenSelected = true; }
  
    bool                  getOptionsItemHasBeenSelected () const
                              { return fOptionsItemHasBeenSelected; }
  
    void                  print (ostream& os) const
      {
        const int fieldWidth = 19;
        
        os <<
          "OptionsItem ???:" <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementShortName" << " : " << fOptionsElementShortName <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementLongName" << " : " << fOptionsElementLongName <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementDescription" << " : " << fOptionsElementDescription <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsItemHasBeenSelected" << " : " << fOptionsItemHasBeenSelected <<
          endl;
      }

  protected:
      
    bool                  fOptionsItemHasBeenSelected;
};
typedef msrOptionsItem* S_msrOptionsItem;

ostream& operator<< (ostream& os, const msrOptionsItem& elt)
{
  os <<
    "OptionsItem ???:" <<
    endl;
  elt.print (os);
  return os;
}

//______________________________________________________________________________
class EXP msrOptionsIntItem : public msrOptionsItem
{
  public:
  
    msrOptionsIntItem (
      string optionsItemShortName,
      string optionsItemLongName,
      string optionsItemDescription,
      int&   optionsIntItemVariable)
      : msrOptionsItem (
          optionsItemShortName,
          optionsItemLongName,
          optionsItemDescription),
        fOptionsIntItemVariable (optionsIntItemVariable)
      {}
      
    void                  print (ostream& os) const
      {
        const int fieldWidth = 19;
        
        os <<
          "OptionsIntItem:" <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementShortName" << " : " << fOptionsElementShortName <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementLongName" << " : " << fOptionsElementLongName <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementDescription" << " : " << fOptionsElementDescription <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsItemHasBeenSelected" << " : " << fOptionsItemHasBeenSelected <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsIntItemVariable" << " : " << fOptionsIntItemVariable <<
          endl;
      }

  private:
  
    int&                  fOptionsIntItemVariable;
};
typedef msrOptionsIntItem* S_msrOptionsIntItem;

ostream& operator<< (ostream& os, const msrOptionsIntItem& elt)
{
  os <<
    "OptionsIntItem:" <<
    endl;
  elt.print (os);
  return os;
}

//______________________________________________________________________________
class EXP msrOptionsStringItem : public msrOptionsItem
{
  public:
  
    msrOptionsStringItem (
      string  optionsItemShortName,
      string  optionsItemLongName,
      string  optionsItemDescription,
      string& optionsStringItemVariable)
      : msrOptionsItem (
          optionsItemShortName,
          optionsItemLongName,
          optionsItemDescription),
        fOptionsStringItemVariable (optionsStringItemVariable)
      {}
      
    void                  print (ostream& os) const
      {
        const int fieldWidth = 19;
        
        os <<
          "OptionsStringItem:" <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementShortName" << " : " << fOptionsElementShortName <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementLongName" << " : " << fOptionsElementLongName <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementDescription" << " : " << fOptionsElementDescription <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsItemHasBeenSelected" << " : " << fOptionsItemHasBeenSelected <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsStringItemVariable" << " : " << fOptionsStringItemVariable <<
          endl;
      }

  private:
  
    string&                  fOptionsStringItemVariable;
};
typedef msrOptionsStringItem* S_msrOptionsStringItem;

ostream& operator<< (ostream& os, const msrOptionsStringItem& elt)
{
  os <<
    "OptionsStringItem:" <<
    endl;
  elt.print (os);
  return os;
}

//_______________________________________________________________________________
class EXP msrOptionsSubGroup : public msrOptionsElement
{
  public:
  
    msrOptionsSubGroup (
      string optionsSubGroupShortName,
      string optionsSubGroupLongName,
      string optionsSubGroupDescription)
      : msrOptionsElement (
          optionsSubGroupShortName,
          optionsSubGroupLongName,
          optionsSubGroupDescription)
      {}
    
    void                  appendOptionsItem (
                            S_msrOptionsItem optionsItem)
      {
        fOptionsSubGroupItemsList.push_back (
          optionsItem);
      }

    void                  print (ostream& os) const
      {
        const int fieldWidth = 19;
        
        os <<
          "OptionsSubGroup:" <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementShortName" << " : " << fOptionsElementShortName <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementLongName" << " : " << fOptionsElementLongName <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementDescription" << " : " << fOptionsElementDescription <<
          endl;
  
        for (
          list<S_msrOptionsItem>::const_iterator
            i = fOptionsSubGroupItemsList.begin();
          i != fOptionsSubGroupItemsList.end();
          i++) {
          // print the element
          os << (*i);
        } // for
      }

  private:
  
    list<S_msrOptionsItem>
                          fOptionsSubGroupItemsList;
};
typedef msrOptionsSubGroup* S_msrOptionsSubGroup;

ostream& operator<< (ostream& os, const msrOptionsSubGroup& elt)
{
  os <<
    "OptionsSubGroup:" <<
    endl;
  elt.print (os);
  return os;
}

//_______________________________________________________________________________
class EXP msrOptionsGroup : public msrOptionsElement
{
  public:
  
    msrOptionsGroup (
      string optionGroupShortName,
      string optionGroupLongName,
      string optionGroupDescription)
      : msrOptionsElement (
          optionGroupShortName,
          optionGroupLongName,
          optionGroupDescription)
      {}
      
    void                  appendOptionsSubGroup (
                            S_msrOptionsSubGroup optionsSubGroup)
      {
        fOptionsGroupSubGroupsList.push_back (
          optionsSubGroup);
      }
        
    void                  print (ostream& os) const
      {
        const int fieldWidth = 19;
        
        os <<
          "OptionsGroup:" <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementShortName" << " : " << fOptionsElementShortName <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementLongName" << " : " << fOptionsElementLongName <<
          endl <<
          setw(fieldWidth) <<
          "fOptionsElementDescription" << " : " << fOptionsElementDescription <<
          endl;
  
        for (
          list<S_msrOptionsSubGroup>::const_iterator
            i = fOptionsGroupSubGroupsList.begin();
          i != fOptionsGroupSubGroupsList.end();
          i++) {
          // print the element
          os << (*i);
        } // for
      }

  private:

    list<S_msrOptionsSubGroup>
                          fOptionsGroupSubGroupsList;
};
typedef msrOptionsGroup* S_msrOptionsGroup;

ostream& operator<< (ostream& os, const msrOptionsGroup& elt)
{
  os <<
    "OptionsGroup:" <<
    endl;
  elt.print (os);
  return os;
}

//______________________________________________________________________________
void optionError (string errorMessage)
{
  cerr <<
    endl <<
    endl <<
    "### ERROR in the options:" <<
    endl <<
    errorMessage <<
    endl <<
    endl;
    
  exit(99);
}

//_______________________________________________________________________________
// a private variable
map<string, string> pOptionShortNames;

void checkOptionUniqueness (
  string optionLongName, string optionShortName)
{
  if (optionShortName == optionLongName) {
    stringstream s;

    s <<
      "option long name '" << optionLongName << "'" <<
      " is also used as short name";
      
    optionError (s.str());
  }
  
  for (
    map<string, string>::iterator i = pOptionShortNames.begin();
    i != pOptionShortNames.end();
    i++) {
      
    // is optionLongName in the options names map?
    if ((*i).first == optionLongName) {
      stringstream s;
  
      s <<
        "option long name '" << optionLongName << "'" <<
        " is specified more that once";
        
      optionError (s.str());
    }

    // is optionShortName in the options names map?
    if ((*i).second == optionShortName) {
      if (optionShortName.size ()) {
        stringstream s;
    
        s <<
          "option short name '" << optionShortName << "'" <<
          " for option long name '" << optionLongName << "'" <<
          " is specified more that once";
          
        optionError (s.str());
      }
    }
  } // for

  // everything OK, register the option names
  pOptionShortNames [optionLongName] = optionShortName;
}

//_______________________________________________________________________________
void analyzeOptions (
  int            argc,
  char*          argv[])
{
  int   n = 0;

  S_msrOptionsGroup optionsGroup =
    new msrOptionsGroup (
    "hog", "helpOptionsGroup", " help for OptionsGroup");

  S_msrOptionsSubGroup optionsSubGroup =
    new msrOptionsSubGroup (
    "hosg", "helpOptionsSubGroup", " help for OptionsSubGroup");

  optionsGroup->
    appendOptionsSubGroup (optionsSubGroup);

  int intVariable = -3;
  S_msrOptionsIntItem optionsItem1 =
    new msrOptionsIntItem (
      "is", "il", "intVariable", intVariable);
  optionsSubGroup->
    appendOptionsItem (optionsItem1);
      
  string stringVariable = "a string";
  S_msrOptionsStringItem optionsItem2 =
    new msrOptionsStringItem (
      "ss", "sl", "stringVariable", stringVariable);
  optionsSubGroup->
    appendOptionsItem (optionsItem2);

  cerr <<
    "optionsItem1:" << endl <<
    optionsItem1 <<
    endl;
    
  cerr <<
    "optionsGroup:" << endl <<
    optionsGroup <<
    endl;
    
  while (true) { 
    if (argv [n] == 0)
      break;

    string currentElement = string (argv [n]);
    
    // print current element
    cout <<
      n << ":" << currentElement <<
      endl;

    // handle current element
    if (currentElement [0] == '-') {
      // this is an option
      string elementTrailer =
        currentElement.substr (1, string::npos);

      /* JMI
      cout <<
        "elementTrailer '" << elementTrailer << "' is preceded by a dash" <<
        endl;
      */

      if (elementTrailer.size ()) {
        if (elementTrailer [0] == '-') {
          // it is a double-dashed option
          string currentDoubleDashedOption =
            elementTrailer.substr (1, string::npos);
          
          cout <<
            "'" << currentDoubleDashedOption << "' is a double-dashed option" <<
            endl;
        }
        else {
          // it is a single-dashed option
          string currentSingleDashedOption =
            elementTrailer; //.substr (1, string::npos);
          
          cout <<
            "'" << currentSingleDashedOption << "' is a single-dashed option" <<
            endl;
        }
      }
      
      else {
        cout <<
          "'-' is the minimal single-dashed option" <<
          endl;
      }
    }

    // next please
    n++;
  } // while
}

int main (int argc, char *argv[])
{
/*
  vector<msrOptionsItem> vec {
    msrOptionItem ("1short", "1long", "descr1"),
    msrOptionItem ("2short", "1long", "descr2")
  };

  int counter = 0;
  cout <<
    "The contents of 'vec' is:" <<
    endl << endl;
  for (msrOptionItem i : vec)
  {
    cout <<
      "Element " << counter << ":" <<
      endl <<
      i <<
      endl;
    counter++;
  }

struct option
{
  const char *name;
  // has_arg can't be an enum because some compilers complain about
  // type mismatches in all the code that assumes it is an int.
  int has_arg;
  int *flag;
  int val;
};
*/

/*
  vector<struct option> myLongOptions {
    option ("1short", no_argument, &vec [0].fOptionSelected, 1),
    option ("1long", no_argument, &vec [0].fOptionSelected, 1),
    
    option ("2short", required_argument, &vec [1].fOptionSelected, 1),
    option ("2long", required_argument, &vec [1].fOptionSelected, 1),
    
    option (0, 0, 0, 0) // option trailer
  };
*/


/*
  cout << "Which element should be printed? ";
  
  int n;
  cin >> n;

  cout << endl;
  
  if (n < vec.size ())
    cout <<
      "Element " << n << " constains:" <<
      endl <<
      endl <<
      vec [n] <<
      endl;
  else
    cout <<
      "Sorry, only elements from 0 to " << vec.size () - 1 << " exist" <<
      endl;
*/

  analyzeOptions (
    argc, argv);

}