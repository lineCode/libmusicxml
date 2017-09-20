/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#include <sstream>

#include <climits>      /* INT_MIN, INT_MAX */
#include <iomanip>      // setw, set::precision, ...

#include "xml2lilypondOptionsHandling.h"

#include "utilities.h"


using namespace std;

namespace MusicXML2 
{

// useful shortcut macros
#define idtr indenter::gIndenter
#define tab  indenter::gIndenter.getSpacer ()

//______________________________________________________________________________
S_xml2lilypondOptionsHandler xml2lilypondOptionsHandler::create (
  string optionHandlerShortName,
  string optionHandlerLongName,
  string optionHandlerDescription)
{
  xml2lilypondOptionsHandler* o = new
    xml2lilypondOptionsHandler (
      optionHandlerShortName,
      optionHandlerLongName,
      optionHandlerDescription);
  assert(o!=0);
  return o;
}

xml2lilypondOptionsHandler::xml2lilypondOptionsHandler (
  string optionHandlerShortName,
  string optionHandlerLongName,
  string optionHandlerDescription)
  : msrOptionsHandler (
      optionHandlerShortName,
      optionHandlerLongName,
      optionHandlerDescription)
{}

xml2lilypondOptionsHandler::~xml2lilypondOptionsHandler()
{}

void xml2lilypondOptionsHandler::initializeOptionsHandler ()
{
}

void xml2lilypondOptionsHandler::print (ostream& os) const
{
  const int fieldWidth = 19;
  
  os <<
    "xml2lilypondOptionsHandler:" <<
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
    list<S_msrOptionsGroup>::const_iterator
      i = fOptionsGroupHandler.begin();
    i != fOptionsGroupHandler.end();
    i++) {
    // print the element
    os << (*i);
  } // for
}

ostream& operator<< (ostream& os, const S_xml2lilypondOptionsHandler& elt)
{
  os <<
    "xml2lilypondOptionsHandler:" <<
    endl;
  elt->print (os);
  return os;
}


}
