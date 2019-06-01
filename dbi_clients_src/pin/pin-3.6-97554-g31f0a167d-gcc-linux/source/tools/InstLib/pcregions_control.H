/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2017 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */

#ifndef PCREGIONS_CONTROL_H
#define PCREGIONS_CONTROL_H

/*! @defgroup CONTROLLER_PCREGIONS
  @ingroup CONTROLLER
   Controller for "regions" that are specified using pc+count.
   Use -pcregions:in pcregions.csv

   Regions are specified using a text file with the records of the form:
   # comment,thread-id,region-id,start-pc,start-pc-count,end-pc,end-pc-count,
     end-pc-relative-count, region-length, region-weight, region-multiplier,
     region-type
   [ fields after the first twelve are ignored, so are any lines beginning 
        with '#' ]
   
   end-pc-relative-count: is the count w.r.t. the beginning of the region.
    [ 
     Corner case: if start-pc==end-pc, the first occurrence of the
        (common) pc is *not* counted in end-pc-relative-count. This
        to account for the fact that when used for relogging, the 
        initial occurrence of the (common) pc will be skipped due to
        delay in region logging {unless '-log:precise_controller' is used}
    ]
    
    region-type : "simulation"|"warmup"
    [ warmup region type contains also the parent region id of the simulation region
    for example - warmup:2
    ]

   Region weight can be computed two ways:
   1. region-weight : number_of_slices_in_cluster/total_number_of_slices
          # Here, all slices are considered equal

   2. alternate-region-weight : number_of_instructions_in_cluster
                                    /total_instruction_count 
          # Here, slices with higher instruction counts contribute more.

    If we use fixed-sized slices, both the weights are the same. However, 
    with variable length intervals, they will be different.

    The "BarrierPoint" paper used the second weight for their prediction. 
    Also, since they were directly computing run-time, the introduced the 
    idea of a region 'multiplier' 
       region-multipler = alternate-region-weight * total_number_of_slices

    That way:
        predicted_runtime = SUM( region_i_runtime * region_i_multiplier)

   Knobs:
   ------
    -pcregions:in foo.csv : input file
    -pcregions:relative  use relative endPC-count 
    -pcregions:merge_warmup  ignore warmupEnd and regionStart
    -pcregions:no_warmup  ignore warmup regions 
    -pcregions:startpc_offset  Add 'offset' to Start PCs of all regions
    -pcregions:image_offset Calculate addresses according to image name and offsets
    -pcregions:verbose : for getting informed about regions/events 
    -pcregions:out : Output file for regions skipped due to overlap 
        The idea is to feed this file with "-pcregions:in" to the next 
        invocation of the tool to process skipped regions.
        * If this knob is specified but no regions are skipped, the output
          file will be empty.

    Region processing:
    -----------------
    * The overall regions picture looks as follows:
        WARMUP--(SIM)REGION

        each sub-region has a start and end event. So there are four 
        events possible (two coinciding . warmup-end and sim-start)
            EVENT_WARMUP_START  : Beginning of warmup region
            EVENT_WARMUP_STOP  : End of warmup region
            EVENT_START        : Beginning of interval
            EVENT_STOP         : End of interval

    * Warmup region needs to specified explicitly in the pcregions file.
      For example, if we are using SimPoint output to generate the pcregion
      file, then the warmup could be N slices prior to the simulation
      region slice. 

*/

#include <algorithm>
#include <sstream> 
#include <string.h>
#include <cctype>
#include "region_utils.H"

using namespace std;
namespace CONTROLLER{

typedef enum { 
    WARMUP_REGION,
    SIMULATION_REGION,
    INVALID_REGION,
}REGION_TYPE;

/*! @ingroup CONTROLLER_PCREGIONS
    This class is used for reading in the pcregions.csv file
*/
class PCREGION
{
    private:   
        friend class CONTROL_PCREGIONS; // allow it to set private fields
        ADDRINT _pcStart; // start pc addr. read in
        ADDRINT _pcEnd; // end pc addr. read in
        UINT64 _countStart; // start count. read in
        UINT64 _countEnd; // end count. read in
        UINT64 _countEndRelative; // end count relative. read in
        UINT64 _length; // region length. read in
        double _weight; // region weight. read in
        double _multiplier; // region multiplier. read in
        string _comment; // region comment. read in
        UINT32 _rid; // region id. read in
        UINT32 _tid; // thread id. read in
        string _startImageName; // start image name. read in
        UINT32 _startImageOffset; // start image offset. read in
        string _endImageName; // end image name. read in
        UINT32 _endImageOffset; // end image offset. read in
        REGION_TYPE _rtype; // region type. assigned
        UINT32 _parentSimulationRid; // N; assigned based on _rtype="warmupN"
        PCREGION * _friendSimulationPCRegion; // assigned 
        UINT32 _weightTimesHundredThousand; // computed + assigned
         // Convert input weight ('double' 0--1)  to  UINT32 to avoid
         // floating point code in the pintools.
        BOOL _overlapWritten;
    public:
        // Constructor
        PCREGION()
        {
            _pcStart = 0; 
            _countStart = 0; 
            _pcEnd = 0; 
            _countEnd = 0; 
            _countEndRelative = 0; 
            _length = 0; 
            _weight = 0; 
            _multiplier = 0; 
            _rid = 0; 
            _tid = 0 ; 
            _startImageOffset = 0;
            _endImageOffset = 0;
            _parentSimulationRid = 0; 
            _friendSimulationPCRegion = NULL; 
            _rtype = INVALID_REGION; 
            _weightTimesHundredThousand = 0;
            _overlapWritten = FALSE;
        }

        // Accessors
        string GetComment() const { return _comment;}
        UINT32 GetRegionId() const { return _rid;}
        BOOL IsWarmupRegion() const { return _rtype==WARMUP_REGION;}
        UINT64 GetRegionStartCount() const { return _countStart; }
        UINT64 GetRegionEndCount() const { return _countEnd; }
        UINT64 GetRegionEndCountRelative() const { return _countEndRelative; }
        UINT64 GetRegionLength() const { return _length; }
        UINT64 GetRegionStartPC() const { return _pcStart; }
        UINT64 GetRegionEndPC() const { return _pcEnd; }
        double GetRegionMultiplier() const { return _multiplier;}
        PCREGION * GetParentSimulationRegion() 
            const { return _friendSimulationPCRegion;}
        UINT32 GetWeightTimesHundredThousand() const {
            return _weightTimesHundredThousand;}
};

typedef vector<PCREGION> PCREGION_VECTOR;


/*! @ingroup CONTROLLER_PCREGIONS
*/

class CONTROL_PCREGIONS
{
    private:
        static const UINT32 BUFSIZE=2000;  

    public:
        CONTROL_PCREGIONS(CONTROL_ARGS & control_args, 
        CONTROL_MANAGER* cm)
        : _control_args(control_args),
          _pcFileKnob(KNOB_MODE_WRITEONCE,
                     control_args.get_knob_family(),
                     "pcregions:in",
                     "",
                     "PC-Regions file",
                     control_args.get_prefix()),
          _pcRelativeKnob(KNOB_MODE_WRITEONCE,
                        control_args.get_knob_family(),
                        "pcregions:relative",
                        "0",
                        "User relative count for endPC",
                        control_args.get_prefix()),
          _pcMergeWarmupKnob(KNOB_MODE_WRITEONCE,
                        control_args.get_knob_family(),
                        "pcregions:merge_warmup",
                        "0",
                        "Ignore warmup-end and the following region-start",
                        control_args.get_prefix()),
          _pcNoWarmupKnob(KNOB_MODE_WRITEONCE,
                        control_args.get_knob_family(),
                        "pcregions:no_warmup",
                        "0",
                        "Ignore warmup regions",
                        control_args.get_prefix()),
          _pcStartPCOffsetKnob(KNOB_MODE_WRITEONCE,
                        control_args.get_knob_family(),
                        "pcregions:startpc_offset",
                        "0",
                        "Add 'offset' to all PCs",
                        control_args.get_prefix()),
          _pcRidKnob(KNOB_MODE_WRITEONCE,
                        control_args.get_knob_family(),
                        "pcregions:rid",
                        "0",
                        "Only trigger region 'rid'.",
                        control_args.get_prefix()),
          _pcVerboseKnob(KNOB_MODE_WRITEONCE,
                        control_args.get_knob_family(),
                        "pcregions:verbose",
                        "0",
                        "Print information about regions/events ",
                        control_args.get_prefix()),
          _imageOffsetKnob(KNOB_MODE_WRITEONCE,
                        control_args.get_knob_family(),
                        "pcregions:image_offset",
                        "0",
                        "The addresses in the input file and image names and offsets",
                        control_args.get_prefix()),
          _pcOutFileKnob(KNOB_MODE_WRITEONCE,
                        control_args.get_knob_family(),
                        "pcregions:out",
                        "",
                        "Output file containing regions skipped due to overlap",
                        control_args.get_prefix())
    {
        _cm = cm;
        _valid = true;
        _maxThreads = PIN_MAX_THREADS;
    }

    /*! @ingroup CONTROLLER_PCREGIONS
      Activate the controller if the -pcregions knob is provided
      @return TRUE if controller can start an interval, otherwise FALSE
    */
    BOOL Activate(BOOL passContext, CHAIN_EVENT_VECTOR ** regionControlChains)
    {
        // If no file is supplied then PC regions class is not activated
        if (strcmp(_pcFileKnob.Value().c_str(),"") == 0)
        {
            regionControlChains  = NULL;
            return FALSE;
        }
        _passContext = passContext;
        _active = true;

        if(_pcRelativeKnob)
        {
            ASSERT((!_pcMergeWarmupKnob), 
                "'relative' knob can not be with 'warmup merge' knob");
        }

        if(_pcRelativeKnob)
        {
            ASSERT((!_imageOffsetKnob), 
                "'relative' knob can not be with 'image offset' knob");
        }

        if(_pcNoWarmupKnob)
        {
            ASSERT((!_pcMergeWarmupKnob), 
                "'no warmup' knob can not be with 'warmup merge' knob");
        }

        Allocate();

        // Read regions from file
        ReadPCRegionsFile();

        // Process regions
        ProcessPCRegions();

        // Verbose prints
        if(_pcVerboseKnob) 
            PrintPCRegions();

        // Copy region controller events
        *regionControlChains = &_regionControlChains;

        return TRUE;
    }
    bool IsActive() const { return _active; };

    PCREGION * LastTriggeredRegion(THREADID tid) const { 
        return _last_triggered_pcregion[tid];}

    // Check if the new event is legal to be fired next
    BOOL inline CheckNewEventLegal (EVENT_TYPE last_event, EVENT_TYPE new_event) {

        if ((last_event == EVENT_INVALID && 
             (new_event == EVENT_START || new_event == EVENT_WARMUP_START)) ||
            (last_event == EVENT_START && new_event == EVENT_STOP) ||
            (last_event == EVENT_WARMUP_START && new_event == EVENT_WARMUP_STOP) || 
            ((last_event == EVENT_STOP || last_event == EVENT_WARMUP_STOP) && 
             (new_event == EVENT_START || new_event == EVENT_WARMUP_START)) ||
             (last_event == EVENT_WARMUP_START && new_event == EVENT_STOP && _pcMergeWarmupKnob) ) 
            return TRUE;
        return FALSE;
    }

    // Check if  we got the stop for the correct PCREGION
    BOOL inline CheckNewPCRegionLegal(PCREGION * last_pcregion, PCREGION * new_pcregion, EVENT_TYPE new_event) {

        // Handle merge warmup 
        if (_pcMergeWarmupKnob && last_pcregion && 
            last_pcregion->_friendSimulationPCRegion == new_pcregion && new_event == EVENT_STOP)
            return TRUE;

        // Check if we are stopping the relevant event
        if ((new_event == EVENT_STOP || new_event == EVENT_WARMUP_STOP) && last_pcregion != new_pcregion)
            return FALSE;
        return TRUE;
    }

    // Get the next region event
    BOOL SetTriggeredRegion(THREADID tid, EVENT_TYPE event_type, VOID* event_handler)  { 

        ASSERT(event_handler, "PCREGION is NULL.");        
        PCREGION * pcregion = (PCREGION *)event_handler;
        
        if (CheckNewEventLegal(_last_fired_event[tid],event_type) &&
            CheckNewPCRegionLegal(_last_triggered_pcregion[tid],pcregion,event_type)) {
            // Save new event
            _last_fired_event[tid] = event_type;
            _last_triggered_pcregion[tid] = pcregion;
            return TRUE;
        } 

        // Write overlapped region to file if needed
        // If this region has a friend region (simulation and warmup)
        // then we need to write it also to the file
        OutputSkippedRegion(tid,pcregion);
        if (pcregion->_friendSimulationPCRegion) {
            OutputSkippedRegion(tid,pcregion->_friendSimulationPCRegion);
        }
        return FALSE;
    }

    private:
        CONTROL_ARGS _control_args;
        bool _valid;
        CONTROLLER::CONTROL_MANAGER* _cm;

        // Allocate control regions data structures
        VOID Allocate()
        {
            _pcregions = new PCREGION_VECTOR[_maxThreads];
            _last_triggered_pcregion = new PCREGION * [_maxThreads];
            memset(_last_triggered_pcregion , 0, 
                   sizeof(_last_triggered_pcregion[0]) * _maxThreads);
            _last_fired_event = new EVENT_TYPE [_maxThreads];
            memset(_last_fired_event , EVENT_INVALID, 
                   sizeof(_last_fired_event[0]) * _maxThreads);
        }

        // Read PC regions from file
        VOID ReadPCRegionsFile()
        {
            // Open file name
            string filename = _pcFileKnob.Value().c_str();
            ifstream rfile(filename.c_str());
            if (!rfile.is_open())
            {
                cerr << "Could not open PCregions file " << 
                    _pcFileKnob.Value().c_str() << endl;
                PIN_ExitApplication(-1);
            }

            // Read records from the file
            UINT32 lineNum = 0;
            UINT32 recordNum = 0;
            PCREGION * pcregion = 0;
            while(true)
            {
                if( rfile.eof() )
                {
                    break;
                }

                CHAR record[BUFSIZE+1];
                CHAR urecord[BUFSIZE+1];
                string field;

                double t_weight;
                string t_comment;
                double t_multiplier;
                INT32 t_rid;
                INT32 t_tid;
                UINT64 t_countStart;
                UINT64 t_countEnd;
                UINT64 t_countEndRelative;
                UINT64 t_length;
                ADDRINT t_pcStart;
                ADDRINT t_pcEnd;
                string t_startImageName;
                UINT32 t_startImageOffset;
                string t_endImageName;
                UINT32 t_endImageOffset;
                string t_typestr;

                // Get the next line
                rfile.getline(record, BUFSIZE);
                lineNum++;

                // Ignore empty lines
                if(strlen(record)==0) 
                    continue;

                // Create a temporary record with lower case letters
                for(UINT32 i=0; i <= strlen(record); i++)
                    urecord[i] = tolower(record[i]); 

                // first word "comment" : this is the header
                if(strncmp(urecord,"comment",7)==0) continue;

                // first letter '#' : this is a comment 
                if(urecord[0]=='#') 
                    continue;

                istringstream s(record);
                recordNum++;

                // Get comment field
                // cerr << "Record # " << recordNum << endl;
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty comment field.");
                t_comment = field;
                // cerr << "Comment " << t_comment << endl;

                // Get thread id field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty thread-id field.");
                t_tid = REGION_UTILS::StringToUINT32(field, "thread-id");
                // cerr << "thread-id " << t_tid << endl;

                // Get region id field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty region-id field.");
                t_rid = REGION_UTILS::StringToUINT32(field, "region-id");
                //cerr << "region-id " << t_rid << endl;

                // Get start PC field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty start-pc field.");
                t_pcStart  = REGION_UTILS::StringToADDRINT(field, 
                                                "simulation-region-start-pc");
                if(_pcStartPCOffsetKnob)  
                {
                    // Get start PC offset if needed
                    //cerr << hex << "start-pc 0x" << t_pcStart  
                    //    << " adding offset 0x"
                    //    << hex << _pcStartPCOffsetKnob; 
                    t_pcStart  += _pcStartPCOffsetKnob;
                    //cerr << " new start-pc 0x" << hex << t_pcStart  << endl;
                }

                // Get start image name field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty start-image-name field.");
                t_startImageName = field;
                //cerr << "start-image-name " << t_startImageName << endl;

                // Get start image offset field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty start-image-offset field.");
                t_startImageOffset  = REGION_UTILS::StringToUINT32(field, 
                                      "start-image-offset",16);
                //cerr << "start-image-offset " << t_startImageOffset << endl;

                // Get start count field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty start-count field.");
                t_countStart  = REGION_UTILS::StringToUINT64(field, 
                    "simulation-region-start-pc-count");
                //cerr << "start-count " << t_countStart << endl;

                // Get end PC field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty end-pc field.");
                t_pcEnd  = REGION_UTILS::StringToADDRINT(field, 
                    "simulation-region-end-pc");
                //cerr << "end-pc " << t_pcEnd << endl;

                // Get end image name field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty end-image-name field.");
                t_endImageName = field;
                //cerr << "end-image-name " << t_endImageName << endl;

                // Get end image offset field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty end-image-offset field.");
                t_endImageOffset  = REGION_UTILS::StringToUINT32(field, 
                                    "end-image-offset",16);
                //cerr << "end-image-offset " << t_endImageOffset << endl;

                // Get end count field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty end-count field.");
                t_countEnd  = REGION_UTILS::StringToUINT64(field, 
                                              "simulation-region-end-pc-count");
                //cerr << "end-count " << t_countEnd << endl;

                // Get relative end count field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty end-count-relative field.");
                t_countEndRelative  = REGION_UTILS::StringToUINT64(field, 
                                   "simulation-region-end-pc-count-relative");
                //cerr << "end-count " << t_countEndRelative << endl;

                // Get length field 
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty length field.");
                t_length  = REGION_UTILS::StringToUINT64(field, 
                                   "simulation-region-length");
                //cerr << "length " << t_length;

                // Get region weight field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty region-weight field.");
                t_weight  = REGION_UTILS::StringToDouble(field, "region-weight");
                ASSERT((t_weight >= 0), 
                        "region-weight (" + field + ") must be positive" );
                ASSERT((t_weight <= 1), 
                        "region-weight (" + field + ") must be between 0 and 1" );
                //cerr << "region-weight" << t_weight << endl;

                // Get region multiplier field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty multiplier field.");
                t_multiplier  = REGION_UTILS::StringToDouble(field, 
                                   "region-multiplier");
                //cerr << "multiplier " << t_multiplier;

                // Get region type field
                field.clear();
                getline(s, field, ',');
                ASSERT(!field.empty(), "Empty region-type field.");
                t_typestr  = field;
                //cerr << "region-type" << t_typestr << endl;


                // Get tail
                string tail;
                s >> tail;
                if(!tail.empty()) 
                {
                    cerr << "WARNING: pcregions file '" << filename << 
                        "' line number " << dec << lineNum << 
                        ": ignoring fields : " << tail  << endl;
                }

                // Create a new PC region class
                _pcregions[t_tid].push_back(PCREGION());
                pcregion = & _pcregions[t_tid].back();
                pcregion->_comment = t_comment;
                pcregion->_rid = t_rid;
                pcregion->_tid = t_tid;
                pcregion->_weight = t_weight;
                pcregion->_multiplier = t_multiplier;
                pcregion->_weightTimesHundredThousand = (UINT32)(t_weight*100000);
                pcregion->_countStart = t_countStart;
                pcregion->_countEnd = t_countEnd;
                pcregion->_countEndRelative = t_countEndRelative;
                pcregion->_length = t_length;
                pcregion->_pcStart = t_pcStart;
                pcregion->_pcEnd = t_pcEnd;
                pcregion->_startImageName = t_startImageName;
                pcregion->_startImageOffset = t_startImageOffset;
                pcregion->_endImageName = t_endImageName;
                pcregion->_endImageOffset = t_endImageOffset;
                if(t_typestr.compare(0,6,"warmup")==0)
                {
                    // Handle warmup region type
                    pcregion->_rtype = WARMUP_REGION;
                    istringstream iss(t_typestr);
                    string tok;
                    UINT32 parentSimulationRid = 0;
                    if ( !getline(iss, tok, ':'))
                    {
                        cerr << "Parent Rid id not embedded for warmup region " 
                            << t_typestr 
                            <<  " region type must be 'warmup:N' " << endl; 
                       PIN_ExitApplication(1);
                    }
                    getline(iss, tok); // Read "N"
                    parentSimulationRid = atoi(tok.c_str()); 
                    if ( parentSimulationRid == 0 )
                    {
                        cerr << "Parent Rid could not be parsed for " 
                            << t_typestr <<  " token " << tok << endl; 
                        PIN_ExitApplication(1);
                    }
                    pcregion->_parentSimulationRid = parentSimulationRid;
                }
                else if(t_typestr == "simulation")
                {
                    // Handle simulation region
                    pcregion->_rtype = SIMULATION_REGION;
                }
                else 
                {
                    cerr << "WARNING: invalid region type" 
                         << t_typestr  
                         << "line number " << dec << lineNum << endl;
                }
            }
            rfile.close();
        }

        // Print PC regions to stderr 
        VOID PrintPCRegions()
        {
            // Loop for all threads
            for(UINT32 tid=0; tid < _maxThreads; tid++)
            {
                // Loop for all PC regions in the thread
                for ( UINT32 i = 0; i < _pcregions[tid].size(); i++ )
                {
                    // Print PC region fields
                    PCREGION * pcregion = & _pcregions[tid][i];
                    cerr << " rid " << pcregion->_rid
                    << " comment '" << pcregion->_comment << "'"
                    << " tid " << pcregion->_tid
                    << " weight " << pcregion->_weight
                    << " weightTimesHundredThousand " 
                    << pcregion->_weightTimesHundredThousand
                    << " multiplier " << pcregion->_multiplier
                    << " pcStart 0x" << hex << pcregion->_pcStart
                    << " countStart " << dec << pcregion->_countStart
                    << " pcEnd 0x" << hex << pcregion->_pcEnd
                    << " countEnd " << dec << pcregion->_countEnd
                    << " countEndRelative " << dec << pcregion->_countEndRelative
                    << " length " << dec << pcregion->_length
                    << " region-type " << ((pcregion->_rtype == WARMUP_REGION) ?
                                        " 'warmup' ": " 'simulation' ")
                    << " parent Rid " << dec << pcregion->_parentSimulationRid
                    << endl;
                }
            }
        }

        // Add an event to the events vector
        VOID InsertOneChain(PCREGION * pcregion)
        {
            // Check if we need to trigger only one region id
            if (_pcRidKnob && _pcRidKnob != pcregion->_rid)
                return;

            // Ignore warmup regions if needed
            if (_pcNoWarmupKnob && pcregion->_rtype == WARMUP_REGION)
                return;

            // Create chain event structure 
            CHAIN_EVENT chain_event;
            chain_event.event_handler = pcregion;
            EVENT_TYPE start_type = EVENT_START;
            EVENT_TYPE end_type = EVENT_STOP;
            if (pcregion->_rtype == WARMUP_REGION) {
                start_type = EVENT_WARMUP_START;
                end_type = EVENT_WARMUP_STOP;
            }

            // Create chain event string for the controller
            if (_pcRelativeKnob){
                chain_event.chain_str = _cm->EventToString(start_type)+":address:"+hexstr(pcregion->_pcStart) +
                    ":count"+decstr(pcregion->_countStart) + ":tid"+decstr(pcregion->_tid) + "," +
                    _cm->EventToString(end_type)+ ":address:"+hexstr(pcregion->_pcEnd) + 
                    ":count"+decstr(pcregion->_countEndRelative) + ":tid"+decstr(pcregion->_tid) ;
                _regionControlChains.push_back(chain_event);
            } 
            else {

                // Add start event in case merge warmup is not used or this is warmup region
                // Or this is simulation region without a friend
                if (!_pcMergeWarmupKnob || pcregion->_rtype == WARMUP_REGION || !pcregion->_friendSimulationPCRegion) 
                {
                    string addr_str;
                    if (_imageOffsetKnob)
                        addr_str = pcregion->_startImageName+"+"+hexstr(pcregion->_startImageOffset);
                    else
                        addr_str = hexstr(pcregion->_pcStart);
                    chain_event.chain_str = _cm->EventToString(start_type)+":address:"+addr_str +
                        ":count"+decstr(pcregion->_countStart) + ":tid"+decstr(pcregion->_tid);
                    _regionControlChains.push_back(chain_event);
                }
                // Add stop event in case merge warmup is not used or this is simulation region 
                // or this is warmup region without a friend
                if (!_pcMergeWarmupKnob || pcregion->_rtype == SIMULATION_REGION || !pcregion->_friendSimulationPCRegion) 
                {
                    string addr_str;
                    if (_imageOffsetKnob)
                        addr_str = pcregion->_endImageName+"+"+hexstr(pcregion->_endImageOffset);
                    else
                        addr_str = hexstr(pcregion->_pcEnd);
                    chain_event.chain_str = _cm->EventToString(end_type)+ ":address:"+addr_str + 
                        ":count"+decstr(pcregion->_countEnd) + ":tid"+decstr(pcregion->_tid) ;
                    _regionControlChains.push_back(chain_event);
                }
            }
        }

        // Connect warmup and simulation regions 
        VOID ConnectWarmupSimulationRegions(PCREGION * warmup_pcregion, UINT32 tid)
        {
            // Loop for all PC regions in the thread
            for ( UINT32 i = 0; i < _pcregions[tid].size(); i++ )
            {
                PCREGION * pcregion = & _pcregions[tid][i];
                if (pcregion->_rid == warmup_pcregion->_parentSimulationRid &&
                    pcregion != warmup_pcregion)
                {
                    pcregion->_friendSimulationPCRegion = warmup_pcregion;
                    warmup_pcregion->_friendSimulationPCRegion = pcregion;
                     
                    // Check if we need to merge warmup and simulation regions
                    if (!_pcMergeWarmupKnob)
                    {
                        // Check that warmup does not end exactly when the real region starts
                        // It is not supported in address alarms
                        if (warmup_pcregion->_pcEnd == pcregion->_pcStart && 
                            warmup_pcregion->_countEnd == pcregion->_countStart) 
                        {
                                ASSERT("FALSE","Warmup and Simulation regions overlap address "+hexstr(warmup_pcregion->_pcEnd));
                        }
                    }
 
                }
            }
        }

        // Check overlapped addresses and counts 
        BOOL FoundOverlap(PCREGION * new_pcregion, UINT32 tid, UINT32 index)
        {
            // Loop for all PC regions in the thread
            for ( UINT32 i = 0; i < index; i++ )
            {
                PCREGION * pcregion = & _pcregions[tid][i];
                if (new_pcregion == pcregion)
                    continue;
                if (_pcMergeWarmupKnob && new_pcregion->_friendSimulationPCRegion == pcregion)
                    continue;

                // Compare PCREGION addresses
                if ((pcregion->_pcStart    == new_pcregion->_pcStart && 
                     pcregion->_countStart == new_pcregion->_countStart) ||
                    (pcregion->_pcStart    == new_pcregion->_pcEnd   && 
                     pcregion->_countStart == new_pcregion->_countEnd) ||
                    (pcregion->_pcEnd      == new_pcregion->_pcEnd   && 
                     pcregion->_countEnd   == new_pcregion->_countEnd) ||
                    (pcregion->_pcEnd      == new_pcregion->_pcStart && 
                     pcregion->_countEnd == new_pcregion->_countStart) )
                {
                    return TRUE;
                }
            }
            return FALSE;
        }

        // Process the PC read from the file
        VOID ProcessPCRegions()
        {
            // Add to controller strings
            // Loop for all threads
            for(UINT32 tid=0; tid < _maxThreads; tid++)
            {

                // Loop for all PC regions in the thread
                // to connect warmup regions and check overlapps
                for ( UINT32 i = 0; i < _pcregions[tid].size(); i++ )
                {
                    PCREGION * pcregion = & _pcregions[tid][i];

                    // Connect warmup region with simulation region
                    if (pcregion->_rtype == WARMUP_REGION) 
                    {
                        ConnectWarmupSimulationRegions(pcregion, tid);
                    }

                    // Check if we got overlapped address in the file
                    if (FoundOverlap(pcregion, tid, i))
                    {
                        // Write overlapped region to file if needed
                        // If this region has a friend region (simulation and warmup)
                        // then we need to write it also to the file
                        OutputSkippedRegion(tid,pcregion);
                        if (pcregion->_friendSimulationPCRegion) {
                            OutputSkippedRegion(tid,pcregion->_friendSimulationPCRegion);
                        }
                    }
                }

                // Loop for all PC regions in the thread to create controller events
                for ( UINT32 i = 0; i < _pcregions[tid].size(); i++ )
                {
                    PCREGION * pcregion = & _pcregions[tid][i];

                    // Generate controller chain with start and stop controller events
                    // If the region is not overlapping 
                    if (!pcregion->_overlapWritten)
                    {
                        InsertOneChain(pcregion);
                    }
                }
            }
        }

        // The major purpose of pcregions controller is to support region
        // logging.
        // Some regions may get skipped due to warmup+simulation overlap as
        // logging of multiple regions at the same time is not supported.
        // This function creates a pcregions.csv file with the skipped region
        // so it can be used for the next iteration of region logging.
        VOID OutputSkippedRegion (THREADID tid, PCREGION* pcregion) 
        {
            if (pcregion->_overlapWritten)
                return;
            pcregion->_overlapWritten = TRUE;
            if (strcmp(_pcOutFileKnob.Value().c_str(),"") == 0) 
                return;

            if (!xfile.is_open())
            {
                xfile.open(_pcOutFileKnob.Value().c_str());
                if (!xfile.is_open())
                {
                    cerr << "Could not open output file " << 
                        _pcOutFileKnob.Value().c_str() << endl;
                    return;
                }
                xfile << "#Generated automatically for skipped pc-regions\n"
                    << "# comment,thread-id,region-id,start-pc,"
                    "start-image-name,start-image-offset,"
                    "start-pc-count,end-pc,"
                    "end-image-name,end-image-offset,"
                    "end-pc-count,end-pc-relative-count,region-length,"
                    "region-weight, region-multiplier, region-type"
                    << endl << endl;
            }

            xfile << "# Region = " << pcregion->_rid << endl;
            xfile << "#Start: pc : 0x" << hex 
                <<  pcregion->_pcStart 
                << " count: " << dec << pcregion->_countStart 
                << endl;
            xfile << "#End: pc : 0x" << hex << pcregion->_pcEnd 
                << " count: " << dec << pcregion->_countEnd 
                << " relative_count: " << dec 
                << pcregion->_countEndRelative << endl;
            xfile << "Skipped region"
                << "," << dec << tid
                << "," << dec << pcregion->_rid
                << ",0x" << hex << pcregion->_pcStart
                << "," << pcregion->_startImageName
                << ",0x" << hex << pcregion->_startImageOffset
                << "," << dec << pcregion->_countStart
                << ",0x" << hex << pcregion->_pcEnd
                << "," << pcregion->_endImageName
                << ",0x" << hex << pcregion->_endImageOffset
                << "," << dec << pcregion->_countEnd
                << "," << dec << pcregion->_countEndRelative
                << "," << dec << pcregion->_length
                << "," << pcregion->_weight
                << "," << pcregion->_multiplier;
            if(pcregion->_rtype == WARMUP_REGION)
            {
                xfile << ",warmup:"  << pcregion->_parentSimulationRid; 
            }
            else
            {
                xfile << ",simulation";
            }
            xfile << endl << endl;
        }

        // Private data members
        KNOB<string> _pcFileKnob;
        KNOB<BOOL> _pcRelativeKnob;
        KNOB<BOOL> _pcMergeWarmupKnob;
        KNOB<BOOL> _pcNoWarmupKnob;
        KNOB<UINT32> _pcStartPCOffsetKnob;
        KNOB<UINT32> _pcRidKnob;
        KNOB<BOOL> _pcVerboseKnob;
        KNOB<BOOL> _imageOffsetKnob;
        KNOB<string> _pcOutFileKnob;
        PCREGION_VECTOR *_pcregions; // per thread vector containing region info
        bool _active;
        THREADID _maxThreads;
        PCREGION ** _last_triggered_pcregion;
        EVENT_TYPE * _last_fired_event;
        BOOL _passContext;
        ofstream xfile;  // for writing out regions excluded due to overlap
        CHAIN_EVENT_VECTOR _regionControlChains;
};
}
#endif
