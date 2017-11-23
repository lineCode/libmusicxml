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

#include <sstream>
#include <climits>      /* INT_MIN */
#include <algorithm>    /* for_each */

#include "conversions.h"

#include "msrOptions.h"
#include "lilypondOptions.h"

#include "xml2lyOptionsHandling.h"

#include "msr2LpsrTranslator.h"


using namespace std;

namespace MusicXML2
{

//________________________________________________________________________
msr2LpsrTranslator::msr2LpsrTranslator (
  indentedOstream& ios,
  S_msrScore            mScore)
    : fLogOutputStream (ios)
{
  // the MSR score we're visiting
  fVisitedMsrScore = mScore;

  // identification
  fOnGoingIdentification = false;

  // header
  fWorkNumberKnown       = false;
  fWorkTitleKnown        = false;
  fMovementNumberKnown   = false;
  fMovementTitleKnown    = false;
   
  // staves
  fOnGoingStaff          = false;

  // voices
  fOnGoingHarmonyVoice     = false;
  fOnGoingFiguredBassVoice = false;

  // repeats
  fOnGoingRepeat         = false;

  // measures
  fMeasuresCounter = 0;
    
  // notes
  fOnGoingNote           = false;

  // double tremolos
  fOnGoingDoubleTremolo  = false;

  // chords
  fOnGoingChord          = false;
  
  // stanzas
  fOnGoingStanza         = false;

  // syllables
  fOnGoingSyllableExtend = false;
};
  
msr2LpsrTranslator::~msr2LpsrTranslator ()
{}

//________________________________________________________________________
void msr2LpsrTranslator::buildLpsrScoreFromMsrScore ()
{
  if (fVisitedMsrScore) {    
    // create a msrScore browser
    msrBrowser<msrScore> browser (this);

    // browse the score with the browser
    browser.browse (*fVisitedMsrScore);
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrScore& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrScore" <<
      endl;
  }

  // create an empty clone of fVisitedMsrScore for use by the LPSR score
  // not sharing the visitiged MSR score allows cleaner data handling
  // and optimisations of the LPSR data
  fCurrentMsrScoreClone =
    fVisitedMsrScore->
      createScoreNewbornClone ();

  // create the LPSR score
  fLpsrScore =
    lpsrScore::create (
      0, // input line number
      fCurrentMsrScoreClone);
      
  // fetch score header
  fLpsrScoreHeader =
    fLpsrScore-> getHeader();


  // is Jianpu notation to be generated?
  if (gLilypondOptions->fJianpu)
    fLpsrScore->
      setJianpuFileIncludeIsNeeded ();

/* JMI
  // push it onto this visitors's stack,
  // making it the current partGroup block
  fPartGroupBlocksStack.push (
    partGroupBlock);
    */
}

void msr2LpsrTranslator::visitEnd (S_msrScore& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrScore" <<
      endl;
  }

  // try to set header title and subtitle
  if (fWorkTitleKnown && fMovementTitleKnown) {
    // we have both the work and movement titles available
    fLpsrScoreHeader->
      changeWorkTitleVariableName ("title");
    fLpsrScoreHeader->
      changeMovementTitleVariableName ("subtitle");
  }
  else if (fWorkTitleKnown) {
    // we only have the work title available
    fLpsrScoreHeader->
      changeWorkTitleVariableName ("title");
  }
  else if (fMovementTitleKnown) {
    // we only have the movement title available
    fLpsrScoreHeader->
      changeMovementTitleVariableName ("title");
  }

  // try to set header subsubtitle
  if (fMovementNumberKnown) {
    fLpsrScoreHeader->
      changeMovementNumberVariableName ("subsubtitle");
  }
  
  // try to set header opus
  if (fWorkNumberKnown) {
    fLpsrScoreHeader->
      changeWorkNumberVariableName ("opus");
  }
  
/* JMI
  // get top level partgroup block from the stack
  S_lpsrPartGroupBlock
    partGroupBlock =
      fPartGroupBlocksStack.top ();

  // pop it from the stack
  fPartGroupBlocksStack.top ();

  // the stack should now be empty
  if (fPartGroupBlocksStack.size ())
    msrInternalError (
      1,
      "the partGroup block stack is not exmpty at the end of the visit");
   */ 
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrIdentification& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrIdentification" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  gIndenter++;

  fOnGoingIdentification = true;
}

void msr2LpsrTranslator::visitEnd (S_msrIdentification& elt)
{
  fOnGoingIdentification = false;
  
  gIndenter--;

  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrIdentification" <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrPageGeometry& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPageGeometry" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  gIndenter++;

  // get LPSR score paper
  S_lpsrPaper
    paper =
      fLpsrScore->getPaper ();

  // populate paper  
  paper ->
    setPaperWidth (elt->getPaperWidth ());
  paper->
    setPaperHeight (elt->getPaperHeight ());
    
  paper->
    setTopMargin (elt->getTopMargin ());
  paper->
    setBottomMargin (elt->getBottomMargin ());
  paper->
    setLeftMargin (elt->getLeftMargin ());
  paper->
    setRightMargin (elt->getRightMargin ());

  // get LPSR score layout
  S_lpsrLayout
    scoreLayout =
      fLpsrScore->getScoreLayout ();

  // get LPSR score global staff size
  float
    globalStaffSize =
      elt->globalStaffSize ();

  // populate layout
  /*
  scoreLayout->
    setMillimeters (elt->getMillimeters ());
  scoreLayout->
    setTenths (elt->getTenths ());
    */

  // populate score global staff size
  fLpsrScore->
    setGlobalStaffSize (globalStaffSize);

  // get LPSR score block layout
  S_lpsrLayout
    scoreBlockLayout =
      fLpsrScore->getScoreBlock ()->getScoreBlockLayout ();

  // create the score block layout staff-size Scheme assoc
  stringstream s;
  s << globalStaffSize;
  S_lpsrSchemeVarValAssoc
    assoc =
      lpsrSchemeVarValAssoc::create (
        0, // JMI
        lpsrSchemeVarValAssoc::kCommented,
        "layout-set-staff-size",
        s.str (),
        "Uncomment and adapt next line as needed (default is 20)",
        lpsrSchemeVarValAssoc::kWithEndl);

  // populate score block layout
  scoreBlockLayout->
    addSchemeVarValAssoc (assoc);

/* JMI
    void    setBetweenSystemSpace (float val) { fBetweenSystemSpace = val; }
    float   getBetweenSystemSpace () const    { return fBetweenSystemSpace; }

    void    setPageTopSpace       (float val) { fPageTopSpace = val; }
   */
}

void msr2LpsrTranslator::visitEnd (S_msrPageGeometry& elt)
{  
  gIndenter--;

  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrPageGeometry" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}


//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrCredit& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrCredit" <<
      endl;
  }

  fCurrentCredit = elt;

  // set elt as credit of the MSR score part of the LPSR score
  fLpsrScore->
    getMsrScore ()->
      appendCreditToScore (fCurrentCredit);
}

void msr2LpsrTranslator::visitEnd (S_msrCredit& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrCredit" <<
      endl;
  }

  fCurrentCredit = nullptr;
}

void msr2LpsrTranslator::visitStart (S_msrCreditWords& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrCreditWords" <<
      endl;
  }

  // don't append it to the current credit, since the latter is no clone
  /* JMI
  fCurrentCredit->
    appendCreditWordsToCredit (
      elt);
      */
}

void msr2LpsrTranslator::visitEnd (S_msrCreditWords& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrCreditWords" <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrPartGroup& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPartGroup " <<
      elt->getPartGroupCombinedName () <<
      endl;
  }

  // create a partGroup clone
  // current partGroup clone, i.e. the top of the stack,
  // is the uplink of the new one if it exists

  S_msrPartGroup
    partGroupClone =
      elt->createPartGroupNewbornClone (
        fPartGroupsStack.size ()
          ? fPartGroupsStack.top ()
          : 0,
        fLpsrScore->getMsrScore ());

  // push it onto this visitors's stack,
  // making it the current partGroup block
  if (gGeneralOptions->fTracePartGroups) {
    fLogOutputStream <<
      "Pushing part group clone " <<
      partGroupClone->getPartGroupCombinedName () <<
      " onto stack" <<
      endl;
  }
  
  fPartGroupsStack.push (
    partGroupClone);
  
/*
  // add it to the MSR score clone
  fCurrentMsrScoreClone->
    addPartGroupToScore (fCurrentPartGroupClone);
*/

  // create a partGroup block refering to the part group clone
  S_lpsrPartGroupBlock
    partGroupBlock =
      lpsrPartGroupBlock::create (
        partGroupClone);

  // push it onto this visitors's stack,
  // making it the current partGroup block
  fPartGroupBlocksStack.push (
    partGroupBlock);
  
  // get the LPSR store block
  S_lpsrScoreBlock
    scoreBlock =
      fLpsrScore->getScoreBlock ();

  // don't append the partgroup block to the score block now:
  // this will be done when it gets popped from the stack
}

void msr2LpsrTranslator::visitEnd (S_msrPartGroup& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrPartGroup " <<
      elt->getPartGroupCombinedName () <<
      endl;
  }

  S_msrPartGroup
    currentPartGroup =
      fPartGroupsStack.top ();
      
  if (fPartGroupsStack.size () == 1) {
    // add the current partgroup clone to the MSR score clone
    // if it is the top-level one, i.e it's alone in the stack
    
    if (gGeneralOptions->fTracePartGroups) {
      fLogOutputStream <<
        "Adding part group clone " <<
        currentPartGroup->getPartGroupCombinedName () <<
        " to MSR score" <<
        endl;
    }

    fCurrentMsrScoreClone->
      addPartGroupToScore (currentPartGroup);

    fPartGroupsStack.pop ();
  }

  else {

    // pop current partGroup from this visitors's stack
    if (gGeneralOptions->fTracePartGroups) {
      fLogOutputStream <<
        "Popping part group clone " <<
        fPartGroupsStack.top ()->getPartGroupCombinedName () <<
        " from stack" <<
        endl;
    }

    fPartGroupsStack.pop ();

    // append the current part group to the one one level higher,
    // i.e. the new current part group
    fPartGroupsStack.top ()->
      appendSubPartGroupToPartGroup (
        currentPartGroup);
  }

  // get the LPSR store block
  S_lpsrScoreBlock
    scoreBlock =
      fLpsrScore->getScoreBlock ();
      
  S_lpsrPartGroupBlock
    currentPartGroupBlock =
      fPartGroupBlocksStack.top ();
      
  if (fPartGroupBlocksStack.size () == 1) {
    // add the current partgroup clone to the LPSR score's parallel music
    // if it is the top-level one, i.e it's alone in the stack
    
    if (gGeneralOptions->fTracePartGroups) {
      fLogOutputStream <<
        "Adding part group block clone for part group " <<
        currentPartGroupBlock->
          getPartGroup ()->
            getPartGroupCombinedName () <<
        " to LPSR score" <<
        endl;
    }

    // append the current partgroup block to the score block
    // if it is the top-level one, i.e it's alone in the stack
   // JMI BOF if (fPartGroupBlocksStack.size () == 1)
      scoreBlock->
        appendPartGroupBlockToParallelMusic (
          fPartGroupBlocksStack.top ());
          
    // pop current partGroup block from this visitors's stack,
    // only now to restore the appearence order
    fPartGroupBlocksStack.pop ();
  }

  else {
    // pop current partGroup block from this visitors's stack
    if (gGeneralOptions->fTracePartGroups) {
      fLogOutputStream <<
        "Popping part group block clone for part group " <<
        currentPartGroupBlock->
          getPartGroup ()->
            getPartGroupCombinedName () <<
        " from stack" <<
        endl;
    }

    fPartGroupBlocksStack.pop ();

    // append the current part group block to the one one level higher,
    // i.e. the new current part group block
    fPartGroupBlocksStack.top ()->
      appendElementToPartGroupBlock (
        currentPartGroupBlock);
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrPart& elt)
{
  string
    partCombinedName =
      elt->getPartCombinedName ();
      
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPart " <<
      partCombinedName <<
      endl;
  }

  if (gGeneralOptions->fTraceParts || gGeneralOptions->fTraceGeneral) {
    fLogOutputStream <<
      endl <<
      "<!--=== part \"" << partCombinedName << "\"" <<
      ", line " << elt->getInputLineNumber () << " ===-->" <<
      endl;
  }

  gIndenter++;

  // create a part clone
  fCurrentPartClone =
    elt->createPartNewbornClone (
      fPartGroupsStack.top ());

  // add it to the partGroup clone
  if (gGeneralOptions->fTraceParts) {
    fLogOutputStream <<
      "Adding part clone " <<
      fCurrentPartClone->getPartCombinedName () <<
      " to part group clone \"" <<
      fPartGroupsStack.top ()->getPartGroupCombinedName () <<
      "\"" <<
      endl;
  }

  fPartGroupsStack.top ()->
    appendPartToPartGroup (fCurrentPartClone);

  // create a part block
  fCurrentPartBlock =
    lpsrPartBlock::create (
      fCurrentPartClone);

  // append it to the current partGroup block
  if (gGeneralOptions->fTraceParts) {
    fLogOutputStream <<
      "Appending part block " <<
      fPartGroupsStack.top ()->getPartGroupCombinedName () <<
      " to stack" <<
      endl;
  }

  fPartGroupBlocksStack.top ()->
    appendElementToPartGroupBlock (fCurrentPartBlock);
}

void msr2LpsrTranslator::visitEnd (S_msrPart& elt)
{
  gIndenter--;

  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrPart " <<
      elt->getPartCombinedName () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrStaffLinesNumber& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStaffLinesNumber" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // create a staff lines number clone
  fCurrentStaffLinesNumberClone =
    elt->
      createStaffLinesNumberNewbornClone ();
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrStaffTuning& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStaffTuning" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
  
  // create a staff tuning clone
  fCurrentStaffTuningClone =
    elt->
      createStaffTuningNewbornClone ();
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrStaffDetails& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStaffDetails" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentStaffLinesNumberClone = nullptr;
  fCurrentStaffTuningClone      = nullptr;
}

void msr2LpsrTranslator::visitEnd (S_msrStaffDetails& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrStaffDetails" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // create a staff details clone
  S_msrStaffDetails
    staffDetailsClone =
      elt->createStaffDetailsNewbornClone (
        fCurrentStaffLinesNumberClone,
        fCurrentStaffTuningClone);

  // append it to the current staff clone
  fCurrentStaffClone->
    appendStaffDetailsToStaff (
      staffDetailsClone);

        
/* JMI
  // add it to the staff clone
  fCurrentStaffClone->
    addStaffTuningToStaff (
      fCurrentStaffTuningClone);

  // create a staff tuning block
  S_lpsrNewStaffTuningBlock
    newStaffTuningBlock =
      lpsrNewStaffTuningBlock::create (
        fCurrentStaffTuningClone->getInputLineNumber (),
        fCurrentStaffTuningClone);

  // append it to the current staff block
  fCurrentStaffBlock->
    appendElementToStaffBlock (newStaffTuningBlock);
    */
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrStaff& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStaff \"" <<
      elt->getStaffName () << "\"" <<
      endl;
  }

  gIndenter++;

  switch (elt->getStaffKind ()) {
    case msrStaff::kMasterStaff:
    case msrStaff::kRegularStaff:
      {
        // create a staff clone
        fCurrentStaffClone =
          elt->createStaffNewbornClone (
            fCurrentPartClone);
          
        // add it to the part clone
        fCurrentPartClone->
          addStaffToPartCloneByItsNumber (
            fCurrentStaffClone);
      
        // create a staff block
        fCurrentStaffBlock =
          lpsrStaffBlock::create (
            fCurrentStaffClone);
      
        string
          partName =
            fCurrentPartClone->getPartName (),
          partAbbreviation =
            fCurrentPartClone->getPartAbbreviation ();
      
        string staffBlockInstrumentName;
        string staffBlockShortInstrumentName;
      
        // don't set instrument name nor short instrument name // JMI
        // if the staff belongs to a piano part where they're already set
        if (! partName.size ())
          staffBlockInstrumentName = partName;
        if (! partAbbreviation.size ())
          staffBlockShortInstrumentName = partAbbreviation;
      
        if (staffBlockInstrumentName.size ())
          fCurrentStaffBlock->
            setStaffBlockInstrumentName (staffBlockInstrumentName);
            
        if (staffBlockShortInstrumentName.size ())
          fCurrentStaffBlock->
            setStaffBlockShortInstrumentName (staffBlockShortInstrumentName);
              
        // append the staff block to the current part block
        fCurrentPartBlock->
          appendElementToPartBlock (fCurrentStaffBlock);
      
        fOnGoingStaff = true;
      }
      break;
      
    case msrStaff::kTablatureStaff:
      // JMI
      {
        // create a staff clone
        fCurrentStaffClone =
          elt->createStaffNewbornClone (
            fCurrentPartClone);
          
        // add it to the part clone
        fCurrentPartClone->
          addStaffToPartCloneByItsNumber (
            fCurrentStaffClone);
      
        // create a staff block
        fCurrentStaffBlock =
          lpsrStaffBlock::create (
            fCurrentStaffClone);
      
        string
          partName =
            fCurrentPartClone->getPartName (),
          partAbbreviation =
            fCurrentPartClone->getPartAbbreviation ();
      
        string staffBlockInstrumentName;
        string staffBlockShortInstrumentName;
      
        // don't set instrument name nor short instrument name // JMI
        // if the staff belongs to a piano part where they're already set
        if (! partName.size ())
          staffBlockInstrumentName = partName;
        if (! partAbbreviation.size ())
          staffBlockShortInstrumentName = partAbbreviation;
      
        if (staffBlockInstrumentName.size ())
          fCurrentStaffBlock->
            setStaffBlockInstrumentName (staffBlockInstrumentName);
            
        if (staffBlockShortInstrumentName.size ())
          fCurrentStaffBlock->
            setStaffBlockShortInstrumentName (staffBlockShortInstrumentName);
              
        // append the staff block to the current part block
        fCurrentPartBlock->
          appendElementToPartBlock (fCurrentStaffBlock);
      
        fOnGoingStaff = true;
      }
      break;
      
    case msrStaff::kPercussionStaff:
      // JMI
      {
        // create a staff clone
        fCurrentStaffClone =
          elt->createStaffNewbornClone (
            fCurrentPartClone);
          
        // add it to the part clone
        fCurrentPartClone->
          addStaffToPartCloneByItsNumber (
            fCurrentStaffClone);
      
        // create a staff block
        fCurrentStaffBlock =
          lpsrStaffBlock::create (
            fCurrentStaffClone);
      
        string
          partName =
            fCurrentPartClone->getPartName (),
          partAbbreviation =
            fCurrentPartClone->getPartAbbreviation ();
      
        string staffBlockInstrumentName;
        string staffBlockShortInstrumentName;
      
        // don't set instrument name nor short instrument name // JMI
        // if the staff belongs to a piano part where they're already set
        if (! partName.size ())
          staffBlockInstrumentName = partName;
        if (! partAbbreviation.size ())
          staffBlockShortInstrumentName = partAbbreviation;
      
        if (staffBlockInstrumentName.size ())
          fCurrentStaffBlock->
            setStaffBlockInstrumentName (staffBlockInstrumentName);
            
        if (staffBlockShortInstrumentName.size ())
          fCurrentStaffBlock->
            setStaffBlockShortInstrumentName (staffBlockShortInstrumentName);
              
        // append the staff block to the current part block
        fCurrentPartBlock->
          appendElementToPartBlock (fCurrentStaffBlock);
      
        fOnGoingStaff = true;
      }
      break;
      
    case msrStaff::kHarmonyStaff:
      {
        // create a staff clone
        fCurrentStaffClone =
          elt->createStaffNewbornClone (
            fCurrentPartClone);
        
        // add it to the part clone
        fCurrentPartClone->
          addStaffToPartCloneByItsNumber (
            fCurrentStaffClone);

        // register it as the part harmony staff
        fCurrentPartClone->
          setPartHarmonyStaff (fCurrentStaffClone);

      /* JMI
        // create a staff block
        fCurrentStaffBlock =
          lpsrStaffBlock::create (
            fCurrentStaffClone);
      
        string
          partName =
            fCurrentPartClone->getPartName (),
          partAbbreviation =
            fCurrentPartClone->getPartAbbreviation ();
      
        string staffBlockInstrumentName;
        string staffBlockShortInstrumentName;
      
        // don't set instrument name nor short instrument name
        // if the staff belongs to a piano part where they're already set
        if (! partName.size ())
          staffBlockInstrumentName = partName;
        if (! partAbbreviation.size ())
          staffBlockShortInstrumentName = partAbbreviation;
      
        if (staffBlockInstrumentName.size ())
          fCurrentStaffBlock->
            setStaffBlockInstrumentName (staffBlockInstrumentName);
            
        if (staffBlockShortInstrumentName.size ())
          fCurrentStaffBlock->
            setStaffBlockShortInstrumentName (staffBlockShortInstrumentName);
              
        // append the staff block to the current part block
        fCurrentPartBlock->
          appendElementToPartBlock (fCurrentStaffBlock);
      */
      
        fOnGoingStaff = true;
      }
      break;
      
    case msrStaff::kFiguredBassStaff:
      {
        // create a staff clone
        fCurrentStaffClone =
          elt->createStaffNewbornClone (
            fCurrentPartClone);
        
        // add it to the part clone
        fCurrentPartClone->
          addStaffToPartCloneByItsNumber (
            fCurrentStaffClone);

        // register it as the part figured bass staff
        fCurrentPartClone->
          setPartFiguredBassStaff (fCurrentStaffClone);

      /* JMI
        // create a staff block
        fCurrentStaffBlock =
          lpsrStaffBlock::create (
            fCurrentStaffClone);
      
        string
          partName =
            fCurrentPartClone->getPartName (),
          partAbbreviation =
            fCurrentPartClone->getPartAbbreviation ();
      
        string staffBlockInstrumentName;
        string staffBlockShortInstrumentName;
      
        // don't set instrument name nor short instrument name
        // if the staff belongs to a piano part where they're already set
        if (! partName.size ())
          staffBlockInstrumentName = partName;
        if (! partAbbreviation.size ())
          staffBlockShortInstrumentName = partAbbreviation;
      
        if (staffBlockInstrumentName.size ())
          fCurrentStaffBlock->
            setStaffBlockInstrumentName (staffBlockInstrumentName);
            
        if (staffBlockShortInstrumentName.size ())
          fCurrentStaffBlock->
            setStaffBlockShortInstrumentName (staffBlockShortInstrumentName);
              
        // append the staff block to the current part block
        fCurrentPartBlock->
          appendElementToPartBlock (fCurrentStaffBlock);
      */
      
        fOnGoingStaff = true;
      }
      break;
  } // switch
}

void msr2LpsrTranslator::visitEnd (S_msrStaff& elt)
{
  gIndenter--;

  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting S_msrStaff \"" <<
      elt->getStaffName () << "\"" <<
      endl;
  }

  switch (elt->getStaffKind ()) {
    case msrStaff::kMasterStaff:
    case msrStaff::kRegularStaff:
      {
        fOnGoingStaff = false;
      }
      break;
      
    case msrStaff::kTablatureStaff:
      // JMI
      break;
      
    case msrStaff::kPercussionStaff:
      // JMI
      break;
      
    case msrStaff::kHarmonyStaff:
      // JMI
      break;
      
    case msrStaff::kFiguredBassStaff:
      // JMI
      break;
  } // switch
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrVoice& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrVoice \"" <<
      elt->getVoiceName () << "\"" <<
      endl;
  }

  int inputLineNumber =
    elt->getInputLineNumber ();
    
  gIndenter++;

  switch (elt->getVoiceKind ()) {
    
    case msrVoice::kMasterVoice:
      msrInternalError (
        gXml2lyOptions->fInputSourceName,
        inputLineNumber,
        __FILE__, __LINE__,
        "a master voice is not expected in msr2LpsrTranslator");
      break;
      
    case msrVoice::kRegularVoice:
      // create a voice clone
      fCurrentVoiceClone =
        elt->createVoiceNewbornClone (
          fCurrentStaffClone);
            
      // add it to the staff clone
      fCurrentStaffClone->
        registerVoiceInStaff (
          inputLineNumber, fCurrentVoiceClone);
    
      // append the voice clone to the LPSR score elements list
      fLpsrScore ->
        appendVoiceToScoreElements (fCurrentVoiceClone);
    
      // append a use of the voice to the current staff block
      fCurrentStaffBlock->
        appendVoiceUseToStaffBlock (fCurrentVoiceClone);
      break;
      
    case msrVoice::kHarmonyVoice:
      {
        /* JMI
        // create the harmony staff and voice if not yet done
        fCurrentPartClone->
          createPartHarmonyStaffAndVoiceIfNotYetDone (
            inputLineNumber);
          
        // fetch harmony voice
        fCurrentVoiceClone =
          fCurrentPartClone->
            getPartHarmonyVoice ();
*/

        // create a voice clone
        fCurrentVoiceClone =
          elt->createVoiceNewbornClone (
            fCurrentStaffClone);
              
        // add it to the staff clone
        fCurrentStaffClone->
          registerVoiceInStaff (
            inputLineNumber, fCurrentVoiceClone);
    
        // register it as the part harmony voice
        fCurrentPartClone->
          setPartHarmonyVoice (fCurrentVoiceClone);

        if (
          elt->getMusicHasBeenInsertedInVoice () // superfluous test ??? JMI
          ) {          
          // append the voice clone to the LPSR score elements list
          fLpsrScore ->
            appendVoiceToScoreElements (
              fCurrentVoiceClone);
      
          // create a ChordNames context command
          string voiceName =
            elt->getVoiceName ();
  
          string partCombinedName =
            elt->fetchVoicePartUplink ()->
              getPartCombinedName ();
                          
          if (gGeneralOptions->fTraceHarmonies) {
            fLogOutputStream <<
              "Creating a ChordNames context for \"" << voiceName <<
              "\" in part " << partCombinedName <<
              endl;
          }
  
          S_lpsrContext
            chordNamesContext =
              lpsrContext::create (
                inputLineNumber,
                lpsrContext::kExistingContextYes,
                lpsrContext::kChordNames,
                voiceName);
  
          // append it to the current part block
          if (gGeneralOptions->fTraceHarmonies) {
            fLogOutputStream <<
              "Appending the ChordNames context for \"" << voiceName <<
              "\" in part " << partCombinedName <<
              endl;
          }
  
          fCurrentPartBlock->
            appendElementToPartBlock (
              chordNamesContext);
  
          fOnGoingHarmonyVoice = true;
        }
      }
      break;
      
    case msrVoice::kFiguredBassVoice:
      {
        /* JMI
        // create the figured bass staff and voice if not yet done
        fCurrentPartClone->
          createPartFiguredBassStaffAndVoiceIfNotYetDone (
            inputLineNumber);
          
        // fetch figured bass voice
        fCurrentVoiceClone =
          fCurrentPartClone->
            getPartFiguredBassVoice ();
*/

        // create a voice clone
        fCurrentVoiceClone =
          elt->createVoiceNewbornClone (
            fCurrentStaffClone);
              
        // add it to the staff clone
        fCurrentStaffClone->
          registerVoiceInStaff (
            inputLineNumber, fCurrentVoiceClone);
    
        // register it as the part figured bass voice
        fCurrentPartClone->
          setPartFiguredBassVoice (fCurrentVoiceClone);

        if (
          elt->getMusicHasBeenInsertedInVoice () // superfluous test ??? JMI
          ) {          
          // append the voice clone to the LPSR score elements list
          fLpsrScore ->
            appendVoiceToScoreElements (
              fCurrentVoiceClone);
      
          // create a FiguredBass context command
          string voiceName =
            elt->getVoiceName ();
  
          string partCombinedName =
            elt->fetchVoicePartUplink ()->
              getPartCombinedName ();
                          
          if (gGeneralOptions->fTraceHarmonies) {
            fLogOutputStream <<
              "Creating a FiguredBass context for \"" << voiceName <<
              "\" in part " << partCombinedName <<
              endl;
          }
  
          S_lpsrContext
            figuredBassContext =
              lpsrContext::create (
                inputLineNumber,
                lpsrContext::kExistingContextYes,
                lpsrContext::kFiguredBass,
                voiceName);
  
          // append it to the current part block
          if (gGeneralOptions->fTraceHarmonies) {
            fLogOutputStream <<
              "Appending the FiguredBass context for \"" << voiceName <<
              "\" in part " << partCombinedName <<
              endl;
          }
  
          fCurrentPartBlock->
            appendElementToPartBlock (
              figuredBassContext);
  
          fOnGoingFiguredBassVoice = true;
        }
      }
      break;
  } // switch

  // clear the voice notes map
  fVoiceNotesMap.clear ();

  fFirstNoteCloneInVoice = nullptr;
}

void msr2LpsrTranslator::visitEnd (S_msrVoice& elt)
{
  gIndenter--;

  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrVoice \"" <<
      elt->getVoiceName () << "\"" <<
      endl;
  }

  switch (elt->getVoiceKind ()) {
    case msrVoice::kMasterVoice:
      msrInternalError (
        gXml2lyOptions->fInputSourceName,
        elt->getInputLineNumber (),
        __FILE__, __LINE__,
        "a master voice is not expected in msr2LpsrTranslator"); // JMI
      break;
      
    case msrVoice::kRegularVoice:
      // JMI
      break;
      
    case msrVoice::kHarmonyVoice:
      fOnGoingHarmonyVoice = false;
      break;
      
    case msrVoice::kFiguredBassVoice:
      fOnGoingFiguredBassVoice = false;
      break;
  } // switch
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrVoiceStaffChange& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrVoiceStaffChange '" <<
      elt->voiceStaffChangeAsString () << "'" <<
      endl;
  }

  // create a voice staff change clone
  S_msrVoiceStaffChange
    voiceStaffChangeClone =
      elt->
        createStaffChangeNewbornClone ();

  // append it to the current voice clone
  fCurrentVoiceClone->
    appendVoiceStaffChangeToVoice (
      voiceStaffChangeClone);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrSegment& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSegment '" <<
      elt->getSegmentAbsoluteNumber () << "'" <<
      endl;
  }

/* JMI

  // fetch the current segment clone
  fCurrentSegmentClone =
    fCurrentVoiceClone->
      getVoiceLastSegment ();
      */

  // create a clone of the segment
  S_msrSegment
    segmentClone =
      elt->createSegmentNewbornClone (
        fCurrentVoiceClone);

  // push it onto the segment clones stack
  fCurrentSegmentClonesStack.push (
    segmentClone);
    
  // append it to the current voice
  fCurrentVoiceClone->
    setVoiceCloneLastSegment ( // cuts link to the one created by default JMI ???
      segmentClone);
}

void msr2LpsrTranslator::visitEnd (S_msrSegment& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSegment '" <<
      elt->getSegmentAbsoluteNumber () << "'" <<
      endl;
  }

  // forget current segment clone
  fCurrentSegmentClonesStack.pop ();
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrHarmony& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrHarmony '" <<
      elt->harmonyAsString () <<
      "'" <<
      endl;
  }

  if (fOnGoingNote) {
    // register the harmony in the current note clone
    fCurrentNoteClone->
      setNoteHarmony (elt);

  // don't append the harmony to the part harmony,
  // this will be done below
  }
  
  else if (fOnGoingChord) {
    // register the harmony in the current chord clone
    fCurrentChordClone->
      setChordHarmony (elt); // JMI
  }
  
  else if (fOnGoingHarmonyVoice) { // JMI
    // register the harmony in the part clone harmony
    fCurrentPartClone->
      appendHarmonyToPartClone (
        fCurrentVoiceClone,
        elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrHarmony& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrHarmony '" <<
      elt->harmonyAsString () <<
      "'" <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrFiguredBass& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrFiguredBass '" <<
      elt->figuredBassAsString () <<
      "'" <<
      endl;
  }

  // create a deep copy of the figured bass
  fCurrentFiguredBass =
    elt->
      createFiguredBassDeepCopy (
        fCurrentPartClone);
  
  if (fOnGoingNote) {
    // register the figured bass in the current note clone
    fCurrentNoteClone->
      setNoteFiguredBass (fCurrentFiguredBass);

  // don't append the figured bass to the part figured bass,
  // this will be done below
  }
  
  else if (fOnGoingChord) {
    // register the figured bass in the current chord clone
    fCurrentChordClone->
      setChordFiguredBass (fCurrentFiguredBass); // JMI
  }
  
  else if (fOnGoingFiguredBassVoice) { // JMI
    // register the figured bass in the part clone figured bass
    fCurrentPartClone->
      appendFiguredBassToPartClone (
        fCurrentVoiceClone,
        fCurrentFiguredBass);
  }
}

void msr2LpsrTranslator::visitStart (S_msrFigure& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrFigure '" <<
      elt->figureAsString () <<
      "'" <<
      endl;
  }

  // append the figure to the current figured bass
  fCurrentFiguredBass->
    appendFiguredFigureToFiguredBass (
      elt);
}

void msr2LpsrTranslator::visitEnd (S_msrFiguredBass& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrFiguredBass '" <<
      elt->figuredBassAsString () <<
      "'" <<
      endl;
  }

  fCurrentFiguredBass = nullptr;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrMeasure& elt)
{    
  int
    inputLineNumber =
      elt->getInputLineNumber ();

  string
    measureNumber =
      elt->getMeasureNumber ();

  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMeasure '" <<
      measureNumber <<
      "'" <<
      ", line " << inputLineNumber <<
      endl;
  }

/* JMI
  {
    fLogOutputStream <<
      endl <<
      elt <<
      endl;
  }
     */
      
  if (gGeneralOptions->fTraceMeasures || gGeneralOptions->fTraceGeneral) {
    fLogOutputStream <<
      endl <<
      "<!--=== measure " << measureNumber <<
      ", line " << inputLineNumber << " ===-->" <<
      endl;
  }

  // measure 1 is created by default initially ??? JMI
  
  // create a clone of the measure
  fCurrentMeasureClone =
    elt->
      createMeasureNewbornClone (
        fCurrentSegmentClonesStack.top ());
      
  // append it to the current segment clone
  fCurrentSegmentClonesStack.top ()->
    appendMeasureToSegment (
      fCurrentMeasureClone);
      
// JMI utile???
  fCurrentPartClone->
    setPartCurrentMeasureNumber (
      measureNumber);

  // should the last bar check's measure be set?
  if (fLastBarCheck) {
    fLastBarCheck->
      setNextBarNumber (
        measureNumber);
      
    fLastBarCheck = nullptr;
  }
}

void msr2LpsrTranslator::finalizeCurrentMeasureClone (
  int          inputLineNumber,
  S_msrMeasure originalMeasure)
{
  // take this measure into account
  fMeasuresCounter++;
  
  // fetch the voice
  S_msrVoice
    voice =
      fCurrentMeasureClone->
        getMeasureSegmentUplink ()->
          getSegmentVoiceUplink ();
    
  // fetch the part measure position high tide
  rational
    partMeasureLengthHighTide = // JMI
      fCurrentMeasureClone->
        fetchMeasurePartUplink ()->
          getPartMeasureLengthHighTide ();

  // get the measure number
  string
    measureNumber =
      fCurrentMeasureClone->
        getMeasureNumber ();

  // get the measure length
  rational
    measureLength =
      fCurrentMeasureClone->
        getMeasureLength ();

  // get the full measure length
  rational
    fullMeasureLength =
      fCurrentMeasureClone->
        getFullMeasureLength ();
    
  if (gGeneralOptions->fTraceMeasures) {
    fLogOutputStream <<
      "Finalizing measure " << measureNumber <<
      " in voice \"" << voice->getVoiceName () <<
      "\", line " << inputLineNumber <<
      endl <<
      "measureLength = " << measureLength <<
      endl <<
      "partMeasureLengthHighTide = " <<
      partMeasureLengthHighTide <<
      endl;
  }

  msrMeasure::msrMeasureKind
    measureKind =
      msrMeasure::kUnknownMeasureKind; // JMI
 // JMI     fMeasureKind = kFullMeasure; // may be changed afterwards
    
  if (measureLength == fullMeasureLength ) {
    // this measure is full
    measureKind =
      msrMeasure::kFullMeasureKind;
  }
      
  else if (measureLength < fullMeasureLength) {
    /*
    if (fSegmentMeasuresList.size () == 1) { // JMI
      // this is the first measure in the segment
      measureKind =
        msrMeasure::kIncompleteLeftMeasure;
    }
    
    else {
      // this is the last measure in the segment
      measureKind =
        msrMeasure::kIncompleteRightMeasure;
    }
    */

    // this measure is an up beat
    measureKind =
      msrMeasure::kUpbeatMeasureKind; // JMI
  }

  else if (measureLength > fullMeasureLength) {
    // this measure is overfull
    measureKind =
      msrMeasure::kOverfullMeasureKind;
  }

  if (false && /* JMI */ measureKind != originalMeasure->getMeasureKind ()) { // JMI
    stringstream s;

    s <<
      "line " << inputLineNumber << ":" <<
      " clone measure:" <<
      endl <<
      fCurrentMeasureClone <<
      endl <<
      "differs for measure kind from original measure:" <<
      endl <<
      originalMeasure;

    msrInternalError (
      gXml2lyOptions->fInputSourceName,
      inputLineNumber,
      __FILE__, __LINE__,
      s.str ());
  }
}

void msr2LpsrTranslator::visitEnd (S_msrMeasure& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMeasure '" <<
      elt->getMeasureNumber () <<
      "'" <<
      ", line " << inputLineNumber <<
      endl;
  }

  string
    measureNumber =
      elt->getMeasureNumber ();

  finalizeCurrentMeasureClone ( // JMI
    inputLineNumber,
    elt); // original measure

  bool doCreateABarCheck = false; // JMI

  switch (elt->getMeasureKind ()) {
    
    case msrMeasure::kUnknownMeasureKind:
      {
        stringstream s;

        s <<
          "measure '" << measureNumber <<
          "' in voice \"" <<
          elt->
            fetchMeasureVoiceUplink ()->
              getVoiceName () <<
          "\" is of unknown kind";

        msrInternalError (
          gXml2lyOptions->fInputSourceName,
          inputLineNumber,
          __FILE__, __LINE__,
          s.str ());
      }
      break;
      
    case msrMeasure::kFullMeasureKind:
      doCreateABarCheck = true;
      break;
      
    case msrMeasure::kUpbeatMeasureKind:
      doCreateABarCheck = true;
      break;
      
    case msrMeasure::kUnderfullMeasureKind:
      doCreateABarCheck = true;
      break;
      
    case msrMeasure::kOverfullMeasureKind:
      doCreateABarCheck = true;
      break;
      
    case msrMeasure::kSenzaMisuraMeasureKind:
      doCreateABarCheck = true;
      break;
      
    case msrMeasure::kEmptyMeasureKind:
      // JMI
      break;
  } // switch

  if (doCreateABarCheck) {
    // create a bar check without next bar number,
    // it will be set upon visitStart (S_msrMeasure&)
    // for the next measure
    fLastBarCheck =
      msrBarCheck::create (
        inputLineNumber);

           /* JMI   
  fLogOutputStream <<
    endl <<
    "***********" <<
    endl <<
    endl;
  fCurrentPartClone->print (fLogOutputStream);
  fLogOutputStream <<
    "***********" <<
    endl <<
    endl;
    */
/* JMI
    // append it to the current voice clone
    fCurrentVoiceClone->
      appendBarCheckToVoice (fLastBarCheck);
      */
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrStanza& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      endl << endl <<
      "--> Start visiting msrStanza \"" <<
      elt->getStanzaName () <<
      "\"" <<
      endl;
  }

  gIndenter++;

//  if (elt->getStanzaTextPresent ()) { // JMI
    fCurrentStanzaClone =
      elt->createStanzaNewbornClone (
        fCurrentVoiceClone);
    
    // append the stanza clone to the LPSR score elements list
    fLpsrScore ->
      appendStanzaToScoreElements (fCurrentStanzaClone);
  
    // append a use of the stanza to the current staff block
    fCurrentStaffBlock ->
      appendLyricsUseToStaffBlock (fCurrentStanzaClone);
//  }
//  else
  //  fCurrentStanzaClone = 0; // JMI

  fOnGoingStanza = true;
}

void msr2LpsrTranslator::visitEnd (S_msrStanza& elt)
{
  gIndenter--;
  
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
        "--> End visiting msrStanza \"" <<
        elt->getStanzaName () <<
        "\"" <<
      endl << // JMI
      endl;
  }

  fOnGoingStanza = true;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrSyllable& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSyllable" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // create the syllable clone
  fCurrentSyllableClone =
    elt->createSyllableNewbornClone (
      fCurrentPartClone);

  // add it to the current stanza clone or current note clone

  if (fOnGoingStanza) // fCurrentStanzaClone JM
    // visiting a syllable as a stanza member
    fCurrentStanzaClone->
      appendSyllableToStanza (fCurrentSyllableClone);
  
  if (fOnGoingNote) { // JMI
    // visiting a syllable as attached to a note
    fCurrentNoteClone->
      appendSyllableToNote (fCurrentSyllableClone);
  }

  // get elt's note uplink
  S_msrNote
    eltNoteUplink =
      fVoiceNotesMap [elt->getSyllableNoteUplink ()];
    
  if (eltNoteUplink) {
    // set syllable clone's note uplink to the clone of elt's note uplink
    if (gGeneralOptions->fTraceLyrics) {
      fLogOutputStream <<
        "Setting syllable note uplink " <<
        fCurrentSyllableClone->syllableAsString () <<
        " to " << eltNoteUplink->noteAsShortString () <<
        endl;
    }

    fCurrentSyllableClone->
      setSyllableNoteUplink (
        eltNoteUplink);
  }
    
  // a syllable ends the sysllable extend range if any
  if (fOnGoingSyllableExtend) {
    // create melisma end command
    S_lpsrMelismaCommand
      melismaCommand =
        lpsrMelismaCommand::create (
          elt->getInputLineNumber (),
          lpsrMelismaCommand::kMelismaEnd);

    // append it to current voice clone
    fCurrentVoiceClone->
      appendOtherElementToVoice (melismaCommand);

    fOnGoingSyllableExtend = false;
  }
}

void msr2LpsrTranslator::visitEnd (S_msrSyllable& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSyllable" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrClef& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrClef" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceClone->
    appendClefToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrClef& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrClef" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrKey& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrKey" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceClone->
    appendKeyToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrKey& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrKey" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTime& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTime" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // append time to voice clone
  fCurrentVoiceClone->
    appendTimeToVoiceClone (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrTime& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTime" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTranspose& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTranspose" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // append transpose to voice clone
  fCurrentVoiceClone->
    appendTransposeToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrTranspose& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTranspose" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTempo& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTempo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceClone->
    appendTempoToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrTempo& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTempo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrArticulation& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrArticulation" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
    // don't add articulations to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addArticulationToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addArticulationToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrArticulation& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrArticulation" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrFermata& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrFermata" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // a fermata is an articulation
  
  if (fOnGoingNote) {
    // don't add articulations to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addArticulationToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addArticulationToChord (elt);
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrArpeggiato& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrArpeggiato" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // an arpeggiato is an articulation
  
  if (fOnGoingNote) {
    // don't add articulations to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addArticulationToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addArticulationToChord (elt);
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrNonArpeggiato& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrNonArpeggiato" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // an nonArpeggiato is an articulation
  
  if (fOnGoingNote) {
    // don't add articulations to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addArticulationToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addArticulationToChord (elt);
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTechnical& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTechnical" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
    // don't add technicals to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addTechnicalToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addTechnicalToChord (elt);
  }

  // doest the score need the 'tongue' function?
  switch (elt->getTechnicalKind ()) {
    case msrTechnical::kArrow:
      break;
    case msrTechnical::kDoubleTongue:
      fLpsrScore->
        setTongueSchemeFunctionIsNeeded ();
      break;
    case msrTechnical::kDownBow:
      break;
    case msrTechnical::kFingernails:
      break;
    case msrTechnical::kHarmonic:
      break;
    case msrTechnical::kHeel:
      break;
    case msrTechnical::kHole:
      break;
    case msrTechnical::kOpenString:
      break;
    case msrTechnical::kSnapPizzicato:
      break;
    case msrTechnical::kStopped:
      break;
    case msrTechnical::kTap:
      break;
    case msrTechnical::kThumbPosition:
      break;
    case msrTechnical::kToe:
      break;
    case msrTechnical::kTripleTongue:
      fLpsrScore->
        setTongueSchemeFunctionIsNeeded ();
      break;
    case msrTechnical::kUpBow:
      break;
  } // switch
}

void msr2LpsrTranslator::visitEnd (S_msrTechnical& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTechnical" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTechnicalWithInteger& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTechnicalWithInteger" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
    // don't add technicalWithIntegers to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addTechnicalWithIntegerToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addTechnicalWithIntegerToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrTechnicalWithInteger& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTechnicalWithInteger" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTechnicalWithString& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTechnicalWithString" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
    // don't add technicalWithStrings to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addTechnicalWithStringToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addTechnicalWithStringToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrTechnicalWithString& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTechnicalWithString" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrOrnament& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrOrnament" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
    // don't add ornaments to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addOrnamentToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addOrnamentToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrOrnament& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrOrnament" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrSingleTremolo& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSingleTremolo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
    // don't add single tremolos to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addSingleTremoloToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addSingleTremoloToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrSingleTremolo& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSingleTremolo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrDoubleTremolo& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrDoubleTremolo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // create a double tremolo clone
  fCurrentDoubleTremoloClone =
    elt->createDoubleTremoloNewbornClone (
      fCurrentVoiceClone);
  
  fOnGoingDoubleTremolo = true;
}

void msr2LpsrTranslator::visitEnd (S_msrDoubleTremolo& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSingleTremolo" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // append the current double tremolo clone to the current voice clone
  fCurrentVoiceClone->
    appendDoubleTremoloToVoice (
      fCurrentDoubleTremoloClone);

  // forget about it
  fCurrentDoubleTremoloClone = nullptr;
  
  fOnGoingDoubleTremolo = false;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrDynamics& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrDynamics" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
    // don't add dynamics to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addDynamicsToNote (elt);

    // is this a non LilyPond native dynamics?
/* JMI
rf = #(make-dynamic-script "rf")
sfpp = #(make-dynamic-script "sfpp")
sffz = #(make-dynamic-script "sffz")
ppppp = #(make-dynamic-script "ppppp")
pppppp = #(make-dynamic-script "pppppp")
fffff = #(make-dynamic-script "fffff")
ffffff = #(make-dynamic-script "ffffff")
*/

    bool knownToLilyPondNatively = true;
    
    switch (elt->getDynamicsKind ()) {
      case msrDynamics::kFFFFF:
      case msrDynamics::kFFFFFF:
      case msrDynamics::kPPPPP:
      case msrDynamics::kPPPPPP:
      case msrDynamics::kRF:
      case msrDynamics::kSFPP:
      case msrDynamics::kSFFZ:
      case msrDynamics::k_NoDynamics:
        knownToLilyPondNatively = false;
          
      default:
        ;
    } // switch
  
    if (! knownToLilyPondNatively) {
      fLpsrScore->
        setDynamicsSchemeFunctionIsNeeded ();   
    }
  }
  
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addDynamicsToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrDynamics& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrDynamics" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrOtherDynamics& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrOtherDynamics" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
    // don't add other dynamics to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addOtherDynamicsToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addOtherDynamicsToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrOtherDynamics& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrOtherDynamics" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrWords& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrWords" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
    // don't add words to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addWordsToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addWordsToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrWords& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrWords" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrSlur& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSlur" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
    // don't add slurs to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addSlurToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addSlurToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrSlur& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrSlur" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrLigature& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrLigature" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
    // don't add ligatures to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addLigatureToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addLigatureToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrLigature& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrLigature" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrWedge& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrWedge" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
    // don't add wedges to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addWedgeToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addWedgeToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrWedge& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrWedge" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrGraceNotes& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrGraceNotes" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  bool doCreateAGraceNoteClone = true;
  
// JMI  bool createSkipGraceNotesInOtherVoices = false;
  
  if (fFirstNoteCloneInVoice) {
    // there is at least a note before these grace notes in the voice
    
    if (
      fCurrentNoteClone->getNoteHasATrill ()
        &&
      fCurrentNoteClone->getNoteIsFollowedByGraceNotes ()) {
      // fPendingAfterGraceNotes already contains
      // the afterGraceNotes to use
      
      if (gGeneralOptions->fTraceGraceNotes) {
        fLogOutputStream <<
          "Optimising grace notes " << 
          elt->graceNotesAsShortString () <<
          "into after grace notes" <<
          endl;
      }
      
      // append the after grace notes to the current voice clone
      fCurrentVoiceClone->
        appendAfterGraceNotesToVoice (
          fPendingAfterGraceNotes);
      
      doCreateAGraceNoteClone = false;
    }
  }

  if (doCreateAGraceNoteClone) {
    // create a clone of this graceNotes
    fCurrentGraceNotesClone =
      elt->
        createGraceNotesNewbornClone (
          fCurrentVoiceClone);
  
    // append it to the current voice clone
    fCurrentVoiceClone->
      appendGraceNotesToVoice (
        fCurrentGraceNotesClone);
  
   // JMI XXL find good criterion for this

    // these grace notes are at the beginning of a segment JMI
//    doCreateAGraceNoteClone = true; // JMI

    // bug 34 in LilyPond should be worked aroud by creating
    // skip grance notes in the other voices of the part

    // create skip graceNotes clone
    if (gGeneralOptions->fTraceGraceNotes) {
      fLogOutputStream <<
        "Creating a skip clone of grace notes " <<
        elt->graceNotesAsShortString () <<
        " to work around LilyPond issue 34" <<
        endl;
    }
  
    S_msrGraceNotes
      skipGraceNotes =
        elt->
          createSkipGraceNotesClone (
            fCurrentVoiceClone);

/* JMI
    // prepend it to the other voices in the part
    prependSkipGraceNotesToPartOtherVoices (
      fCurrentPartClone,
      fCurrentVoiceClone,
      skipGraceNotes);
*/

    // append it to the other voices in the part
    fCurrentPartClone->
      appendSkipGraceNotesToVoicesClones (
        fCurrentVoiceClone,
        skipGraceNotes);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrGraceNotes& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrGraceNotes" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // forget about these grace notes if any
  fCurrentGraceNotesClone = nullptr;

  if (fPendingAfterGraceNotes) {
    // remove the current afterGraceNotes note clone
    // from the current voice clone
    fCurrentVoiceClone->
      removeNoteFromVoice (
        elt->getInputLineNumber (),
        fCurrentAfterGraceNotesNote);
        
    fCurrentAfterGraceNotesNote = nullptr;
  
    // forget these after grace notes if any
    fPendingAfterGraceNotes = nullptr;
  }
}

void msr2LpsrTranslator::prependSkipGraceNotesToPartOtherVoices (
  S_msrPart       partClone,
  S_msrVoice      voiceClone,
  S_msrGraceNotes skipGraceNotes)
{
    if (gGeneralOptions->fTraceGraceNotes) {
      fLogOutputStream <<
        "--> prepending a skip graceNotes clone " <<
        skipGraceNotes->graceNotesAsShortString () <<
        " to voices other than \"" <<
        voiceClone->getVoiceName () << "\"" <<
        " in part " <<
        partClone->getPartCombinedName () <<
        endl;
    }
  
  map<int, S_msrStaff>
    partStavesMap =
      partClone->
        getPartStavesMap ();

  for (
    map<int, S_msrStaff>::const_iterator i=partStavesMap.begin ();
    i!=partStavesMap.end ();
    i++) {

    map<int, S_msrVoice>
      staffAllVoicesMap =
        (*i).second->
          getStaffAllVoicesMap ();
          
    for (
      map<int, S_msrVoice>::const_iterator j=staffAllVoicesMap.begin ();
      j!=staffAllVoicesMap.end ();
      j++) {

      S_msrVoice voice = (*j).second;
      
      if (voice != voiceClone) {
        // prepend skip grace notes to voice
        voice->
          prependGraceNotesToVoice (
            skipGraceNotes);
      }
    } // for

  } // for
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrNote& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrNote " <<
      elt->noteAsString () <<
      endl;
  }

  int inputLineNumber =
    elt->getInputLineNumber ();
    
  // create the clone
  fCurrentNoteClone =
    elt->createNoteNewbornClone (
      fCurrentPartClone);

  // register clone in this tranlastors' voice notes map
  fVoiceNotesMap [elt] = fCurrentNoteClone; // JMI XXL
  
  if (! fFirstNoteCloneInVoice)
    fFirstNoteCloneInVoice =
      fCurrentNoteClone;

  // can we optimize graceNotes into afterGraceNotes?
  if (
    elt->getNoteHasATrill ()
      &&
    elt->getNoteIsFollowedByGraceNotes ()) {
      
    // yes, create the after grace notes
    fPendingAfterGraceNotes =
      msrAfterGraceNotes::create (
        inputLineNumber,
        fCurrentNoteClone,
        false, // aftergracenoteIsSlashed, may be updated later
        fCurrentVoiceClone);

    // register current afterGraceNotes note
    fCurrentAfterGraceNotesNote =
      fCurrentNoteClone;
  }

  fOnGoingNote = true;
}

void msr2LpsrTranslator::visitEnd (S_msrNote& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrNote " <<
      elt->noteAsString () <<
      endl;
  }

  int inputLineNumber =
    elt->getInputLineNumber ();
    
  switch (fCurrentNoteClone->getNoteKind ()) {
    
    case msrNote::k_NoNoteKind:
      break;
      
    case msrNote::kRestNote:
      if (gGeneralOptions->fTraceNotes) {
        fLogOutputStream <<
          "Appending " <<
          fCurrentNoteClone->noteAsString () << " to voice clone " <<
          fCurrentVoiceClone->getVoiceName () <<
          endl;
      }
          
      fCurrentVoiceClone->
        appendNoteToVoiceClone (
          fCurrentNoteClone);
      break;
      
    case msrNote::kSkipNote: // JMI
      if (gGeneralOptions->fTraceNotes) {
        fLogOutputStream <<
          "Appending " <<
          fCurrentNoteClone->noteAsString () << " to voice clone " <<
          fCurrentVoiceClone->getVoiceName () <<
          endl;
      }
          
      fCurrentVoiceClone->
        appendNoteToVoiceClone (
          fCurrentNoteClone);
      break;
      
    case msrNote::kStandaloneNote:
      if (gGeneralOptions->fTraceNotes) {
        fLogOutputStream <<
          "Appending " <<
          fCurrentNoteClone->noteAsString () << " to voice clone " <<
          fCurrentVoiceClone->getVoiceName () <<
          endl;
      }
          
      fCurrentVoiceClone->
        appendNoteToVoiceClone (
          fCurrentNoteClone);
      break;
      
    case msrNote::kDoubleTremoloMemberNote:
      if (fOnGoingDoubleTremolo) {
        
        if (fCurrentNoteClone->getNoteIsFirstNoteInADoubleTremolo ()) {
          if (gGeneralOptions->fTraceNotes) {
            fLogOutputStream <<
              "Setting standalone note '" <<
              fCurrentNoteClone->noteAsString () <<
              "', line " << fCurrentNoteClone->getInputLineNumber () <<
              ", as double tremolo first element" <<
              " in voice \"" <<
              fCurrentVoiceClone->getVoiceName () <<
              "\"" <<
              endl;
          }
              
          fCurrentDoubleTremoloClone->
            setDoubleTremoloNoteFirstElement (
              fCurrentNoteClone);
        }
        
        else if (fCurrentNoteClone->getNoteIsSecondNoteInADoubleTremolo ()) {
          if (gGeneralOptions->fTraceNotes) {
            fLogOutputStream <<
              "Setting standalone note '" <<
              fCurrentNoteClone->noteAsString () <<
              "', line " << fCurrentNoteClone->getInputLineNumber () <<
              ", as double tremolo second element" <<
              " in voice \"" <<
              fCurrentVoiceClone->getVoiceName () <<
              "\"" <<
              endl;
          }
              
          fCurrentDoubleTremoloClone->
            setDoubleTremoloNoteSecondElement (
              fCurrentNoteClone);
        }
        
        else {
          stringstream s;

          s <<
            "note '" << fCurrentNoteClone->noteAsShortString () <<
            "' belongs to a double tremolo, but is not marked as such";

          msrInternalError (
            gXml2lyOptions->fInputSourceName,
            inputLineNumber,
            __FILE__, __LINE__,
            s.str ());
        }
      }

      else {
        stringstream s;

        s <<
          "double tremolo note '" << fCurrentNoteClone->noteAsShortString () <<
          "' met outside of a double tremolo";

        msrInternalError (
          gXml2lyOptions->fInputSourceName,
          inputLineNumber,
          __FILE__, __LINE__,
          s.str ());
      }
      break;
      
    case msrNote::kGraceNote:
      if (fCurrentGraceNotesClone) {
        if (gGeneralOptions->fTraceGraceNotes || gGeneralOptions->fTraceNotes) {
          fLogOutputStream <<
            "Appending note " <<
            fCurrentNoteClone->notePitchAsString () <<
            ":" <<
            fCurrentNoteClone->getNoteSoundingWholeNotes () <<
            " to the grace notes in voice \"" <<
            fCurrentVoiceClone->getVoiceName () << "\"" <<
            endl;
        }
  
        fCurrentGraceNotesClone->
          appendNoteToGraceNotes (
            fCurrentNoteClone);
      }

      else if (fPendingAfterGraceNotes) {
        if (gGeneralOptions->fTraceGraceNotes || gGeneralOptions->fTraceNotes) {
          fLogOutputStream <<
            "Appending note " <<
            fCurrentNoteClone->notePitchAsString () <<
            ":" <<
            fCurrentNoteClone->getNoteSoundingWholeNotes () <<
            " to the after grace notes in voice \"" <<
            fCurrentVoiceClone->getVoiceName () << "\"" <<
            endl;
        }
  
        fPendingAfterGraceNotes->
          appendNoteToAfterGraceNotesContents (
            fCurrentNoteClone);
      }
      
      else {
        stringstream s;

        s <<
          "both fCurrentGraceNotesClone and fPendingAfterGraceNotes are null," <<
          endl <<
          "cannot handle grace note'" <<
          elt->noteAsString () <<
          "'";

        msrInternalError (
          gXml2lyOptions->fInputSourceName,
          inputLineNumber,
          __FILE__, __LINE__,
          s.str ());
      }
      break;
      
    case msrNote::kChordMemberNote:
      if (fOnGoingChord) {
        fCurrentChordClone->
          addAnotherNoteToChord (
            fCurrentNoteClone);
      }
      
      else {
        stringstream s;

        s <<
          "msr2LpsrTranslator:::visitEnd (S_msrNote& elt): chord member note " <<
          elt->noteAsString () <<
          " appears outside of a chord";

        msrInternalError (
          gXml2lyOptions->fInputSourceName,
          inputLineNumber,
          __FILE__, __LINE__,
          s.str ());
        }
      break;
      
    case msrNote::kTupletMemberNote:
      if (gGeneralOptions->fTraceNotes) {
        fLogOutputStream <<
          "Appending " <<
          fCurrentNoteClone->noteAsString () << " to voice clone " <<
          fCurrentVoiceClone->getVoiceName () <<
          endl;
      }
          
      fTupletClonesStack.top ()->
        addNoteToTuplet (fCurrentNoteClone);
      break;
  } // switch

  // handle editorial accidentals
  switch (fCurrentNoteClone->getNoteEditorialAccidentalKind ()) {
    case msrNote::kNoteEditorialAccidentalYes:
      fLpsrScore->
        setEditorialAccidentalSchemeFunctionIsNeeded ();
      break;
    case msrNote::kNoteEditorialAccidentalNo:
      break;
  } // switch
  
  // handle cautionary accidentals
  switch (fCurrentNoteClone->getNoteCautionaryAccidentalKind ()) {
    case msrNote::kNoteCautionaryAccidentalYes:
      break;
    case msrNote::kNoteCautionaryAccidentalNo:
      break;
  } // switch

  // handle melisma
  msrSyllable::msrSyllableExtendKind
    noteSyllableExtendKind =
      elt->getNoteSyllableExtendKind ();

  switch (noteSyllableExtendKind) {
    case msrSyllable::kStandaloneSyllableExtend:
      {
        // create melisma start command
        S_lpsrMelismaCommand
          melismaCommand =
            lpsrMelismaCommand::create (
              elt->getInputLineNumber (),
              lpsrMelismaCommand::kMelismaStart);
    
        // append it to current voice clone
        fCurrentVoiceClone->
          appendOtherElementToVoice (melismaCommand);

        // append 

        fOnGoingSyllableExtend = true;
      }
      break;
    case msrSyllable::kStartSyllableExtend:
      break;
    case msrSyllable::kContinueSyllableExtend:
      break;
    case msrSyllable::kStopSyllableExtend:
      break;
    case msrSyllable::k_NoSyllableExtend:
      break;
  } // switch

  fOnGoingNote = false;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrOctaveShift& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrOctaveShift" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceClone->
    appendOctaveShiftToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrOctaveShift& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrOctaveShift" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrAccordionRegistration& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrAccordionRegistration" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // append the accordion registration to the voice clone
  fCurrentVoiceClone->
    appendAccordionRegistrationToVoice (elt);

  // the generated code needs modules scm and accreg
  fLpsrScore->
    setScmAndAccregSchemeModulesAreNeeded ();
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrHarpPedalsTuning& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrHarpPedalsTuning" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // append the harp pedals tuning to the voice clone
  fCurrentVoiceClone->
    appendHarpPedalsTuningToVoice (elt);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrStem& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrStem" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
 //   // don't add stems to chord member notes JMI ???
 //   if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        setNoteStem (elt);
  }
  /* JMI
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addStemToChord (elt);
  }
  */
}

void msr2LpsrTranslator::visitEnd (S_msrStem& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrStem" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrBeam& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrBeam" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingNote) {
    // don't add beams to chord member notes
    if (fCurrentNoteClone->getNoteKind () != msrNote::kChordMemberNote)
      fCurrentNoteClone->
        addBeamToNote (elt);
  }
  else if (fOnGoingChord) {
    fCurrentChordClone->
      addBeamToChord (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrBeam& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrBeam" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrChord& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrChord" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentChordClone =
    elt->createChordNewbornClone (
      fCurrentPartClone);

  if (fTupletClonesStack.size ()) {
    // a chord in a tuplet is handled as part of the tuplet JMI
    fTupletClonesStack.top ()->
      addChordToTuplet (
        fCurrentChordClone);
  }

  else if (fOnGoingDoubleTremolo) {
    if (elt->getChordIsFirstChordInADoubleTremolo ()) {
      // replace double tremolo's first element by chord
      fCurrentDoubleTremoloClone->
        setDoubleTremoloChordFirstElement (
          elt);
    }
    
    else if (elt->getChordIsSecondChordInADoubleTremolo ()) {
      // replace double tremolo's second element by chord
      fCurrentDoubleTremoloClone->
        setDoubleTremoloChordSecondElement (
          elt);
    }
    
    else {
      stringstream s;

      s <<
        "chord '" << elt->chordAsString () <<
        "' belongs to a double tremolo, but is not marked as such";

      msrInternalError (
        gXml2lyOptions->fInputSourceName,
        elt->getInputLineNumber (),
        __FILE__, __LINE__,
        s.str ());
    }
  }
  
  else {
    // appending the chord to the voice clone at once
    fCurrentVoiceClone->
      appendChordToVoice (
        fCurrentChordClone);
  }

  fOnGoingChord = true;
}

void msr2LpsrTranslator::visitEnd (S_msrChord& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrChord" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fOnGoingChord = false;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTuplet& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTuplet" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // create the tuplet clone
  S_msrTuplet
    tupletClone =
      elt->createTupletNewbornClone ();

  // register it in this visitor
  if (gGeneralOptions->fTraceTuplets) {
    fLogOutputStream <<
      "++> pushing tuplet '" <<
      tupletClone->tupletAsString () <<
      "' to tuplets stack" <<
      endl;
  }
  
  fTupletClonesStack.push (tupletClone);

  switch (elt->getTupletLineShapeKind ()) {
    case msrTuplet::kTupletLineShapeStraight:
    case msrTuplet::kTupletLineShapeCurved:
      fLpsrScore->
        setTupletsCurvedBracketsSchemeFunctionIsNeeded ();   
      break;
  } // switch
}

void msr2LpsrTranslator::visitEnd (S_msrTuplet& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTuplet" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (gGeneralOptions->fTraceTuplets) {
    fLogOutputStream <<
      "Popping tuplet '" <<
      elt->tupletAsString () <<
      "' from tuplets stack" <<
      endl;
  }
      
  fTupletClonesStack.pop ();

  if (fTupletClonesStack.size ()) {
    // tuplet is a nested tuplet
    if (gGeneralOptions->fTraceTuplets) {
      fLogOutputStream <<
        "Adding nested tuplet '" <<
      elt->tupletAsString () <<
        "' to stack top tuplet '" <<
      fTupletClonesStack.top ()->tupletAsString () <<
      "'" <<
      endl;
    }
    
    fTupletClonesStack.top ()->
      addTupletToTupletClone (elt);
  }
  
  else {
    // tuplet is a top level tuplet
    
    if (gGeneralOptions->fTraceTuplets) {
      fLogOutputStream <<
        "Adding top level tuplet '" <<
      elt->tupletAsString () <<
      "' to voice" <<
      fCurrentVoiceClone->getVoiceName () <<
      endl;
    }
      
    fCurrentVoiceClone->
      appendTupletToVoice (elt);
  }  
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrTie& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrTie" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // we don't need the kTieStop or kContinue for LilyPond
  if (elt->getTieKind () == msrTie::kStartTie) {
    fCurrentNoteClone->
      setNoteTie (elt);
  }
}

void msr2LpsrTranslator::visitEnd (S_msrTie& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrTie" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrSegno& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrSegno" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceClone->
    appendSegnoToVoice (elt);
}

void msr2LpsrTranslator::visitStart (S_msrCoda& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrCoda" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceClone->
    appendCodaToVoice (elt);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrEyeGlasses& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting eyeGlasses" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceClone->
    appendEyeGlassesToVoice (elt);
}

void msr2LpsrTranslator::visitStart (S_msrPedal& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting pedal" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceClone->
    appendPedalToVoice (elt);
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrBarCheck& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrBarCheck" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceClone->
    appendBarCheckToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrBarCheck& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrBarCheck" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrBarNumberCheck& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrBarNumberCheck" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceClone->
    appendBarNumberCheckToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrBarNumberCheck& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrBarNumberCheck" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrLineBreak& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrLineBreak" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceClone->
    appendLineBreakToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrLineBreak& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrLineBreak" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrPageBreak& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrPageBreak" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceClone->
    appendPageBreakToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrPageBreak& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrPageBreak" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrRepeat& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRepeat" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

/* JMI
  // create a repeat clone
  if (gGeneralOptions->fTraceRepeats) {
    fLogOutputStream <<
      "Creating a repeat newborn clone" <<
      ", line " << elt->getInputLineNumber () <<
      ", in voice \"" <<
      elt->
        getRepeatVoiceUplink ()->
          getVoiceName () <<
      "\"" <<
      endl;
  }

  fCurrentRepeatClone =
    elt->
      createRepeatNewbornClone (
        fCurrentVoiceClone);
*/
        
  fOnGoingRepeat = true; // JMI
}

void msr2LpsrTranslator::visitEnd (S_msrRepeat& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRepeat" <<
      ", line " << inputLineNumber <<
      endl;
  }

/* JMI
  // append the repeat clone to the current part clone
  if (gGeneralOptions->fTraceRepeats) {
    fLogOutputStream <<
      "Appending a repeat to part clone " <<
      fCurrentPartClone->getPartCombinedName () << "\"" <<
      endl;
  }

  fCurrentPartClone-> // no test needed JMI
    appendRepeatCloneToPart (
      inputLineNumber,
      fCurrentRepeatClone);
*/

  // forget about current repeat clone // JMI
// JMI  fCurrentRepeatClone = 0;
  
  fOnGoingRepeat = false;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrRepeatCommonPart& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRepeatCommonPart" <<
      endl;
  }
}

void msr2LpsrTranslator::visitEnd (S_msrRepeatCommonPart& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRepeatCommonPart" <<
      endl;
  }

  // create a repeat and append it to voice clone
  if (gGeneralOptions->fTraceRepeats) {
    fLogOutputStream <<
      "Appending a repeat  to voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }

 // JMI fCurrentRepeatClone =
  fCurrentVoiceClone->
    createRepeatUponItsEndAndAppendItToVoice ( // JMI
      elt->getInputLineNumber (),
      elt->
        getRepeatCommonPartRepeatUplink ()->
          getRepeatTimes ());
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrRepeatEnding& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRepeatEnding" <<
      endl;
  }
}

void msr2LpsrTranslator::visitEnd (S_msrRepeatEnding& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRepeatEnding" <<
      endl;
  }

  // create a repeat ending clone and append it to voice clone
  if (gGeneralOptions->fTraceRepeats) {
    fLogOutputStream <<
      "Appending a repeat ending clone to voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }

  fCurrentVoiceClone->
    appendRepeatEndingToVoice (
      elt->getInputLineNumber (),
      elt->getRepeatEndingNumber (),
      elt->getRepeatEndingKind ());
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrMeasuresRepeat& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMeasuresRepeat" <<
      endl;
  }

  gIndenter++;
}

void msr2LpsrTranslator::visitEnd (S_msrMeasuresRepeat& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMeasuresRepeat" <<
      endl;
  }

  gIndenter--;

  // set last segment as the measure repeat pattern segment
  if (gGeneralOptions->fTraceRepeats) {
    fLogOutputStream <<
      "Setting current last segment as measure repeat pattern segment in voice \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrMeasuresRepeatPattern& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMeasuresRepeatPattern" <<
      endl;
  }

  gIndenter++;
}

void msr2LpsrTranslator::visitEnd (S_msrMeasuresRepeatPattern& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMeasuresRepeatPattern" <<
      endl;
  }

  gIndenter--;

  // get the measures repeat uplink
  S_msrMeasuresRepeat
    measuresRepeat =
      elt->getMeasuresRepeatUplink ();

  // create a measures repeat and append it to voice clone
  if (gGeneralOptions->fTraceRepeats) {
    fLogOutputStream <<
      "Appending a measures repeat to voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }

  fCurrentVoiceClone->
    createMeasuresRepeatAndAppendItToVoiceClone (
      elt->getInputLineNumber (),
      measuresRepeat->
        getMeasuresRepeatMeasuresNumber (),
      measuresRepeat->
        getMeasuresRepeatSlashesNumber ());

  // forget about the current measure repeat pattern clone
  fCurrentMeasuresRepeatPatternClone = nullptr;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrMeasuresRepeatReplicas& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMeasuresRepeatReplicas" <<
      endl;
  }

  gIndenter++;
}

void msr2LpsrTranslator::visitEnd (S_msrMeasuresRepeatReplicas& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMeasuresRepeatReplicas" <<
      endl;
  }

  gIndenter--;

  // create a measures repeat replica clone and append it to voice clone
  if (gGeneralOptions->fTraceRepeats) {
    fLogOutputStream <<
      "Appending a repeat replica clone to voice clone \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }

  fCurrentVoiceClone->
    appendMeasuresRepeatReplicaToVoice (
      elt->getInputLineNumber ());

  // forget about the current measure repeat replicas clone
 // JMI ??? fCurrentMeasuresRepeatReplicasClone = nullptr;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrMultipleRest& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMultipleRest" <<
      endl;
  }
}

void msr2LpsrTranslator::visitEnd (S_msrMultipleRest& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMultipleRest" <<
      endl;
  }
  
  // create the multiple rest clone
  S_msrMultipleRest
    multipleRestClone =
      elt->createMultipleRestNewbornClone (
        fCurrentVoiceClone);

  // set the multiple rest clone's contents
  multipleRestClone->
    setMultipleRestContents (
      fCurrentMultipleRestContentsClone);
    
  // create a new last segment to collect the remainder of the voice,
  // containing the next, yet incomplete, measure
  if (gGeneralOptions->fTraceSegments || gGeneralOptions->fTraceVoices) {
    fLogOutputStream <<
      "Creating a new last segment for the remainder of voice \"" <<
      fCurrentVoiceClone->getVoiceName () << "\"" <<
      endl;
  }

  fCurrentVoiceClone->
    createNewLastSegmentForVoice (
      elt->getInputLineNumber ());

  // append the multiple rest clone to the current part clone
  fCurrentPartClone->
    appendMultipleRestCloneToPart (
      elt->getInputLineNumber (), // JMI ???
      multipleRestClone);
      
  // forget about the current multiple rest contents clone
  fCurrentMultipleRestContentsClone = nullptr;
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrMultipleRestContents& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMultipleRestContents" <<
      endl;
  }

  gIndenter++;

  // create a new last segment to collect the multiple rest contents
  if (gGeneralOptions->fTraceSegments || gGeneralOptions->fTraceVoices) {
    fLogOutputStream <<
      "Creating a new last segment for a multiple rest contents for voice \"" <<
      fCurrentVoiceClone->getVoiceName () << "\"" <<
      endl;
  }
      
  fCurrentVoiceClone->
    createNewLastSegmentForVoice (
      elt->getInputLineNumber ());
}

void msr2LpsrTranslator::visitEnd (S_msrMultipleRestContents& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMultipleRestContents" <<
      endl;
  }

  gIndenter--;

  // create a multiple rest contents clone
  fCurrentMultipleRestContentsClone =
    elt->createMultipleRestContentsNewbornClone (
      fCurrentVoiceClone);

  // set last segment as the multiple rest contents segment
  if (gGeneralOptions->fTraceRepeats) {
    fLogOutputStream <<
      "Setting current last segment as multiple rest contents segment in voice \"" <<
      fCurrentVoiceClone->getVoiceName () <<
      "\"" <<
      endl;
  }

  fCurrentMultipleRestContentsClone->
    setMultipleRestContentsSegment (
      fCurrentVoiceClone->
        getVoiceLastSegment ());
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrBarline& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();
    
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrBarline" <<
      ", line " << inputLineNumber <<
      endl;
  }

  switch (elt->getBarlineCategory ()) {
    
    case msrBarline::kBarlineCategoryStandalone:
      {
        if (gGeneralOptions->fTraceRepeats) {
          fLogOutputStream <<
            "Handling kStandaloneBarline in voice \"" <<
            fCurrentVoiceClone->getVoiceName () << "\"" <<
            endl;
        }

        // append the barline to the current voice clone
        fCurrentVoiceClone->
          appendBarlineToVoice (elt);
      }
      break;
      
    case msrBarline::kBarlineCategoryRepeatStart:
      {
        if (gGeneralOptions->fTraceRepeats) {
          fLogOutputStream <<
            "Handling kRepeatStart in voice \"" <<
            fCurrentVoiceClone->getVoiceName () << "\"" <<
            endl;
        }
    
        // append the barline to the current voice clone
        fCurrentVoiceClone->
          appendBarlineToVoice (elt);
      }
      break;

    case msrBarline::kBarlineCategoryRepeatEnd:
      {
        if (gGeneralOptions->fTraceRepeats) {
          fLogOutputStream <<
            "Handling kRepeatEnd in voice " <<
            fCurrentVoiceClone->getVoiceName () << "\"" <<
            endl;
        }
  
        // append the barline to the current voice clone
        fCurrentVoiceClone->
          appendBarlineToVoice (elt);
/*
        // append the repeat clone to the current part clone
        if (gGeneralOptions->fTraceRepeats) {
          fLogOutputStream <<
            "Appending a repeat to part clone " <<
            fCurrentPartClone->getPartCombinedName () << "\"" <<
            endl;
        }
  
        fCurrentPartClone-> // no test needed JMI
          createRepeatAndAppendItToPart (
            inputLineNumber);
            */
        }
      break;
            
    case msrBarline::kBarlineCategoryHookedEndingStart:
      {
        // append the barline to the current voice clone
        fCurrentVoiceClone->
          appendBarlineToVoice (elt);
      }
      break;
      
    case msrBarline::kBarlineCategoryHookedEndingEnd:
      {
        if (gGeneralOptions->fTraceRepeats) {
          fLogOutputStream <<
            "Handling kHookedEndingEnd in voice \"" <<
            fCurrentVoiceClone->getVoiceName () << "\"" <<
            endl;
        }
  
        // append the barline to the current voice clone
        fCurrentVoiceClone->
          appendBarlineToVoice (elt);

    /* JMI
        if (gGeneralOptions->fTraceRepeats) {
          fLogOutputStream <<
            "Appending a hooked repeat ending clone to voice clone \"" <<
            fCurrentVoiceClone->getVoiceName () << "\"" <<
            endl;
        }

        fCurrentPartClone->
          appendRepeatEndingToPart (
            inputLineNumber,
            elt->getEndingNumber (),
            msrRepeatEnding::kHookedEnding);
            */
      }
      break;
      
    case msrBarline::kBarlineCategoryHooklessEndingStart:
      {
        if (gGeneralOptions->fTraceRepeats) {
          fLogOutputStream <<
            "Handling kHooklessEndingStart in voice \"" <<
            fCurrentVoiceClone->getVoiceName () << "\"" <<
            endl;
        }
  
        // append the barline to the current voice clone
        fCurrentVoiceClone->
          appendBarlineToVoice (elt);
      }
      break;

    case msrBarline::kBarlineCategoryHooklessEndingEnd:
      {
        if (gGeneralOptions->fTraceRepeats) {
          fLogOutputStream <<
            "Handling kHooklessEndingEnd in voice \"" <<
            fCurrentVoiceClone->getVoiceName () << "\"" <<
            endl;
        }
  
        // append the barline to the current voice clone
        fCurrentVoiceClone->
          appendBarlineToVoice (elt);

/* JMI
        if (gGeneralOptions->fTraceRepeats) {
          fLogOutputStream <<
            "Appending a hookless repeat ending clone to voice clone \" " <<
            fCurrentVoiceClone->getVoiceName () << "\"" <<
            endl;
        }

        fCurrentPartClone->
          appendRepeatEndingToPart (
            inputLineNumber,
            elt->getEndingNumber (),
            msrRepeatEnding::kHookedEnding);
            */
      }
      break;
  } // switch
}

void msr2LpsrTranslator::visitEnd (S_msrBarline& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrBarline" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrVarValAssoc& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrVarValAssoc" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  string variableName     = elt->getVariableName ();
  string variableValueAux = elt->getVariableValue ();
  int    inputLineNumber  = elt->getInputLineNumber ();
  string variableValue;

  // escape quotes if any
  for_each (
    variableValueAux.begin (),
    variableValueAux.end (),
    stringQuoteEscaper (variableValue));

  if      (variableName == "work-number") {
    fLpsrScoreHeader->
      setWorkNumber (
        inputLineNumber, variableValue);

    fWorkNumberKnown = true;
  }
  
  else if (variableName == "work-title") {
    fLpsrScoreHeader->
      setWorkTitle (
        inputLineNumber, variableValue);
        
    fWorkTitleKnown = true;
  }
  
  else if (variableName == "movement-number") {
    fLpsrScoreHeader->
      setMovementNumber (
        inputLineNumber, variableValue);

    fMovementNumberKnown = true;
  }
  
  else if (variableName == "movement-title") {
    fLpsrScoreHeader->
      setMovementTitle (
        inputLineNumber, variableValue);
        
    fMovementTitleKnown = true;
  }
  
  else if (variableName == "composer") {
    S_lpsrLilypondVarValAssoc
      assoc =
        fLpsrScoreHeader->
          addComposer (
            inputLineNumber, variableName, variableValue);
  }
  
  else if (variableName == "arranger") {
    S_lpsrLilypondVarValAssoc
      assoc =
        fLpsrScoreHeader->
          addArranger (
            inputLineNumber, variableName, variableValue);
  }
  
  else if (variableName == "poet") {
    S_lpsrLilypondVarValAssoc
      assoc =
        fLpsrScoreHeader->
          addLyricist ( // JMI
            inputLineNumber, "poet", variableValue);
  }

  else if (variableName == "lyricist") {
    S_lpsrLilypondVarValAssoc
      assoc =
        fLpsrScoreHeader->
          addLyricist (
            inputLineNumber, "lyricist", variableValue); // JMI ???
  }

  else if (variableName == "rights") {
    fLpsrScoreHeader->setRights (
        inputLineNumber, variableValue);

    fLpsrScoreHeader->
      changeRightsTitleVariableName ("copyright");
  }
  
  else if (variableName == "software") {
    fLpsrScoreHeader->addSoftware (
      inputLineNumber, variableValue);
  }
  
  else if (variableName == "encoding-date") {
    fLpsrScoreHeader->setEncodingDate (
      inputLineNumber, variableValue);
  }

  else if (variableName == "miscellaneous-field") {
    fLpsrScoreHeader->setMiscellaneousField (
      inputLineNumber, variableValue);
  }

  else {
    stringstream s;

    s <<
      "### msrVarValAssoc name '" << variableName << "'" <<
      " is not handled";

    msrMusicXMLWarning (
      gXml2lyOptions->fInputSourceName,
      elt->getInputLineNumber (),
      s.str ());
  }
}

void msr2LpsrTranslator::visitEnd (S_msrVarValAssoc& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrVarValAssoc" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrLayout& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrLayout" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  gIndenter++;
}

void msr2LpsrTranslator::visitEnd (S_msrLayout& elt)
{
  gIndenter--;

  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrLayout" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrMidi& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrMidi" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

void msr2LpsrTranslator::visitEnd (S_msrMidi& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrMidi" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//________________________________________________________________________
void msr2LpsrTranslator::visitStart (S_msrRehearsal& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> Start visiting msrRehearsal" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceClone->
    appendRehearsalToVoice (elt);
}

void msr2LpsrTranslator::visitEnd (S_msrRehearsal& elt)
{
  if (gMsrOptions->fTraceMsrVisitors) {
    fLogOutputStream <<
      "--> End visiting msrRehearsal" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}


} // namespace


/* JMI
 *   fCurrentVoiceClone =
    elt->createVoiceNewbornClone (fCurrentStaffClone);
    
  fCurrentStaffClone->
    registerVoiceInStaff (fCurrentVoiceClone);

  // append the voice to the LPSR score elements list
  fLpsrScore ->
    appendVoiceToScoreElements (fCurrentVoiceClone);

  // append the voice use to the LPSR score block
  fLpsrScore ->
    appendVoiceUseToStoreBlock (fCurrentVoiceClone);
*/





           /* JMI   
  fLogOutputStream <<
    endl <<
    "*********** fCurrentPartClone" <<
    endl <<
    endl;
  fCurrentPartClone->print (fLogOutputStream);
  fLogOutputStream <<
    "*********** fCurrentPartClone" <<
    endl <<
    endl;
    */
/* JMI
  fLogOutputStream <<
    endl <<
    "*********** fCurrentRepeatClone" <<
    endl <<
    endl;
  fCurrentRepeatClone->print (fLogOutputStream);
  fLogOutputStream <<
    "*********** fCurrentRepeatClone" <<
    endl <<
    endl;
*/

