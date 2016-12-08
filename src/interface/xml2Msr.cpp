/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#ifdef VC6
# pragma warning (disable : 4786)
#endif

#include <iostream>

#include "xml.h"
#include "xmlfile.h"
#include "xmlreader.h"
// JMI #include "xml_tree_browser.h"

#include "versions.h"

#include "xml2Msr.h"

#include "xml2MsrVisitor.h"
#include "msr2SummaryVisitor.h"

using namespace std;

namespace MusicXML2 
{

//_______________________________________________________________________________
EXP int         musicxml2MsrVersionNbr () { return 010; }
EXP const char* musicxml2MsrVersionStr () { return "0.1.0"; }

//_______________________________________________________________________________
static S_msrScore xml2Msr (
  SXMLFile&     xmlfile,
  S_msrOptions& msrOpts,
  ostream&      os,
  const char*   file) 
{
  // build xmlelement tree from the file contents
  Sxmlelement elemsTree = xmlfile->elements();

  S_msrScore mScore;
  
  if (elemsTree) {
    mScore =
      buildMsrScoreFromElementsTree (msrOpts, elemsTree);

    if (msrOpts->fDisplayMSR)
      displayMsrScore (msrOpts, mScore, os);

    if (msrOpts->fDisplayMSRScoreSummary)
      displayMsrScoreSummary (msrOpts, mScore, os);
  }

  return mScore;
}

//_______________________________________________________________________________
EXP S_msrScore musicxmlFile2Msr (
  const char*   file,
  S_msrOptions& msrOpts,
  ostream&      os) 
{
  if (msrOpts->fTrace)
    cerr << idtr <<
      "Pass 1: building xmlemement tree from \"" << file << "\"" << endl;
      // ------------------------------------------------------

  xmlreader r;
  SXMLFile  xmlFile;

  xmlFile = r.read (file);
  
  S_msrScore mScore;

  if (xmlFile)
    mScore = xml2Msr (xmlFile, msrOpts, os, file);
  
  return mScore;
}

//_______________________________________________________________________________
EXP S_msrScore musicxmlFd2Msr (
  FILE*         fd,
  S_msrOptions& msrOpts,
  ostream&      os) 
{
  if (msrOpts->fTrace)
    cerr << idtr <<
      "Pass 1: building xmlemement tree from standard input" << endl;
      // ------------------------------------------------------

  xmlreader r;
  SXMLFile  xmlFile;

  xmlFile = r.read (fd);
  
  S_msrScore mScore;

  if (xmlFile) {
    mScore = xml2Msr (xmlFile, msrOpts, os, 0);
  }
  
  return mScore;
}

//_______________________________________________________________________________
EXP S_msrScore musicxmlString2Msr (
  const char*   buffer,
  S_msrOptions& msrOpts,
  ostream&      os) 
{
  if (msrOpts->fTrace)
    cerr << idtr <<
      "Pass 1: building xmlemement tree from a buffer" << endl;
      // ------------------------------------------------------
  
  xmlreader r;
  SXMLFile  xmlFile;
  
  xmlFile = r.readbuff (buffer);
  
  S_msrScore mScore;

  if (xmlFile) {
    mScore = xml2Msr (xmlFile, msrOpts, os, 0);
  }
  
  return mScore;
}

//_______________________________________________________________________________
S_msrScore buildMsrScoreFromElementsTree (
  S_msrOptions& msrOpts,
  Sxmlelement   xmlTree)
{
    // browse the part contents for the first time with a xml2MsrVisitor
  if (msrOpts->fTrace)
    cerr << idtr <<
      "Pass 2: building a MSR from the xmlelement tree" << endl;
      // ------------------------------------------------------

  idtr++;
  
  // create an xml2MsrVisitor
  xml2MsrVisitor visitor (msrOpts);

  // build the MSR score
  S_msrScore
    mScore =
      visitor.buildMsrScoreFromXMLElementTree (xmlTree);

  idtr--;

  return mScore;
}

//_______________________________________________________________________________
void displayMsrScore (
  S_msrOptions& msrOpts,
  S_msrScore    mScore,
  ostream&      os)
{
  string separator = "%----------------------------------------";

  // output the score resulting from the conversion 
  if (msrOpts->fTrace) 
    os << idtr <<
      separator << endl <<
      "%Pass 2 bis: outputting a view of the MSR" << endl <<
      separator << endl;
  
  if (msrOpts->fTrace) os << "%{" << endl;
  os << mScore;
  if (msrOpts->fTrace) os << "%}" << endl;
  
  os << separator << endl;
}

//_______________________________________________________________________________
void displayMsrScoreSummary (
  S_msrOptions& msrOpts,
  S_msrScore    mScore,
  ostream&      os)
{
  string separator = "%----------------------------------------";

  // output the score resulting from the conversion 
  if (msrOpts->fTrace) 
    os << idtr <<
      separator << endl <<
      "%Outputting a summary of the MSR" << endl <<
      separator << endl;
   
  // create an msr2SummaryVisitor visitor
  msr2SummaryVisitor visitor (msrOpts, os);

  if (msrOpts->fTrace) os << "%{" << std::endl;
  
  visitor.printSummaryFromMsrScore (mScore);
  
  if (msrOpts->fTrace) os << "%}" << std::endl;
  
  os << separator << std::endl;
}


}
