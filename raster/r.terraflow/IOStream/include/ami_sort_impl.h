/*C
 * Original project: Lars Arge, Jeff Chase, Pat Halpin, Laura Toma, Dean
 *                   Urban, Jeff Vitter, Rajiv Wickremesinghe 1999
 * 
 * GRASS Implementation: Lars Arge, Helena Mitasova, Laura Toma 2002
 *
 * Copyright (c) 2002 Duke University -- Laura Toma 
 *
 * Copyright (c) 1999-2001 Duke University --
 * Laura Toma and Rajiv Wickremesinghe
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Duke University
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE TRUSTEES AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE TRUSTEES OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *C*/



#ifndef AMI_SORT_IMPL_H
#define AMI_SORT_IMPL_H

#include "ami_stream.h"
#include "mem_stream.h"
#include "mm.h"
#include "quicksort.h"
#include "queue.h"
#include "replacementHeap.h"
#include "replacementHeapBlock.h"

#define SDEBUG if(0)


/* if this flag is defined, a run will be split into blocks, each
   block sorted and then all blocks merged */
#define BLOCKED_RUN 



/* ---------------------------------------------------------------------- */
//set run_size, last_run_size and nb_runs depending on how much memory
//is available
template<class T>
static void 
initializeRunFormation(AMI_STREAM<T> *instream,
		       size_t &run_size, size_t &last_run_size, 
		       unsigned int &nb_runs) {
  
  off_t strlen = instream->stream_len();  
  size_t mm_avail = MM_manager.memory_available();
#ifdef BLOCKED_RUN
  // not in place, can only use half memory 
  mm_avail = mm_avail/2;
#endif
  
  run_size = mm_avail/sizeof(T);

  if (strlen == 0) {
    nb_runs = last_run_size = 0;
  } else {
    if (strlen % run_size == 0) {
      nb_runs = strlen/run_size;
      last_run_size = run_size;
    } else {
      nb_runs = strlen/run_size + 1;
      last_run_size = strlen % run_size;
    }
  }
  SDEBUG cout << "nb_runs=" << nb_runs 
	      << ", run_size=" << run_size 
	      << ", last_run_size=" << last_run_size
	      << "\n"; 
}



/* ---------------------------------------------------------------------- */
/* data is allocated; read run_size elements from stream into data and
   sort them using quicksort */
template<class T, class Compare>
void makeRun_Block(AMI_STREAM<T> *instream, T* data, 
		   unsigned int run_size, Compare *cmp) {
  AMI_err err;
  
  //read next run from input stream
  err = instream->read_array(data, run_size); 
  assert(err == AMI_ERROR_NO_ERROR);
  
  //sort it in memory in place
  quicksort(data, run_size, *cmp);
  
}


/* ---------------------------------------------------------------------- */
/* data is allocated; read run_size elements from stream into data and
   sort them using quicksort; instead of reading the whole chunk at
   once, it reads it in blocks, sorts each block and then merges the
   blocks together. Note: it is not in place! it allocates another
   array of same size as data, writes the sorted run into it and
   deteles data, and replaces data with outdata */
template<class T, class Compare>
void makeRun(AMI_STREAM<T> *instream, T* &data, 
	     int run_size, Compare *cmp) {
  
  unsigned int nblocks, last_block_size, crt_block_size, block_size; 

  block_size = STREAM_BUFFER_SIZE;

  if (run_size % block_size == 0) {
    nblocks = run_size / block_size;
    last_block_size = block_size;
  } else { 
    nblocks = run_size / block_size + 1;
    last_block_size = run_size % block_size;
  }
  
  //create queue of blocks waiting to be merged
  queue<MEM_STREAM<T> *> *blockList;
  MEM_STREAM<T>* str;
  blockList  = new  queue<MEM_STREAM<T> *>(nblocks);
  for (unsigned int i=0; i < nblocks; i++) {
    crt_block_size = (i == nblocks-1) ? last_block_size: block_size;
    makeRun_Block(instream, &(data[i*block_size]), crt_block_size, cmp);
    str = new MEM_STREAM<T>( &(data[i*block_size]), crt_block_size);
    blockList->enqueue(str);
  }
  assert(blockList->length() == nblocks);
  
  //now data consists of sorted blocks: merge them 
  ReplacementHeapBlock<T,Compare> rheap(blockList);
  SDEBUG rheap.print(cerr);
  int i = 0;
  T elt;  
  T* outdata = new T [run_size];
  while (!rheap.empty()) {
    elt = rheap.extract_min();
    outdata[i] = elt; 
    //SDEBUG cerr << "makeRun: written " << elt << endl;
    i++;
  }
  assert( i == run_size &&  blockList->length() == 0);
  delete blockList;
 
  T* tmp = data; 
  delete [] tmp;
  data = outdata;
}



/* ---------------------------------------------------------------------- */

//partition instream in streams that fit in main memory, sort each
//stream, remember its name, make it persistent and store it on
//disk. if entire stream fits in memory, sort it and store it and
//return it.

//assume instream is allocated prior to the call.
// set nb_runs and allocate runNames.

//The comparison object "cmp", of (user-defined) class represented by
//Compare, must have a member function called "compare" which is used
//for sorting the input stream.  

template<class T, class Compare>
queue<char*>*
runFormation(AMI_STREAM<T> *instream, Compare *cmp) {
 
  size_t run_size,last_run_size, crt_run_size, nb_runs;
  queue<char*>* runList;
  T* data;
  AMI_STREAM<T>* str;
  char* strname;
  
  assert(instream && cmp);
  SDEBUG cout << "runFormation: ";
  SDEBUG MM_manager.print();
  
  //rewind file
  instream->seek(0); //should check error xxx
  
  //estimate run_size, last_run_size and nb_runs
  initializeRunFormation(instream, run_size, last_run_size, nb_runs);
  
  //create runList
  runList = new queue<char*>(nb_runs);
  
  //allocate space for a run
  if (nb_runs <= 1) {
    //don't waste space if input stream is smaller than run_size
    data = new T[last_run_size];
  } else {
    data = new T[run_size];
  }
  SDEBUG MM_manager.print();
  
  for (size_t i=0; i< nb_runs; i++) {
    
    crt_run_size = (i == nb_runs-1) ? last_run_size: run_size;
    
    //SDEBUG cout << "i=" << i << ":  runsize=" << crt_run_size << ", ";  

#ifdef BLOCKED_RUN
    makeRun(instream, data, crt_run_size, cmp);
#else        
    makeRun_Block(instream, data, crt_run_size, cmp);
#endif
    SDEBUG MM_manager.print();

    //read next run from input stream
    //err = instream->read_array(data, crt_run_size); 
    //assert(err == AMI_ERROR_NO_ERROR);
    //sort it in memory in place
    //quicksort(data, crt_run_size, *cmp);
    
    //create a new stream to hold this run 
    str = new AMI_STREAM<T>();
    str->write_array(data, crt_run_size);
    assert(str->stream_len() == crt_run_size);
    
    //remember this run's name
    str->name(&strname);
    runList->enqueue(strname);
    
    //delete the stream -- should not keep too many streams open
    str->persist(PERSIST_PERSISTENT);
    delete str;
  };
  SDEBUG MM_manager.print();
  //release the run memory!
  delete [] data;
  
  SDEBUG cout << "runFormation: done.\n";
  SDEBUG MM_manager.print();

  return runList;
};






/* ---------------------------------------------------------------------- */

//this is one pass of merge; estimate max possible merge arity <ar>
//and merge the first <ar> streams from runList ; create and return
//the resulting stream (does not add it to the queue -- the calling
//function will do that)

//input streams are assumed to be sorted, and are not necessarily of
//the same length.

//streamList does not contains streams, but names of streams, which
//must be opened in order to be merged

//The comparison object "cmp", of (user-defined) class represented by
//Compare, must have a member function called "compare" which is used
//for sorting the input stream.  


template<class T, class Compare>
AMI_STREAM<T>* 
singleMerge(queue<char*>* streamList, Compare *cmp) {
  AMI_STREAM<T>* mergedStr;
  size_t mm_avail, blocksize;
  unsigned int arity, max_arity; 
  T elt;

  assert(streamList && cmp);

  SDEBUG cout << "singleMerge: ";

  //estimate max possible merge arity with available memory (approx M/B)
  mm_avail = MM_manager.memory_available();
  blocksize = getpagesize();
  //should use AMI function, but there's no stream at this point
  // AMI_STREAM<T>::main_memory_usage(&blocksize, MM_STREAM_USAGE_BUFFER);
  max_arity = mm_avail/blocksize;
  arity = (streamList->length() < max_arity) ? 
    streamList->length() :  max_arity;

  SDEBUG cout << "arity=" << arity << " (max_arity=" <<max_arity<< ")\n"; 
  
  //create output stream
  mergedStr = new AMI_STREAM<T>();
  
  ReplacementHeap<T,Compare> rheap(arity, streamList);
  SDEBUG rheap.print(cerr);

  int i = 0;
  while (!rheap.empty()) {
    //mergedStr->write_item( rheap.extract_min() );
    //xxx should check error here
    elt = rheap.extract_min();
    mergedStr->write_item(elt);
    //SDEBUG cerr << "smerge: written " << elt << endl;
    i++;
  }
  
  SDEBUG cout << "..done\n";

  return mergedStr;
}




/* ---------------------------------------------------------------------- */

//merge runs whose names are given by runList; this may entail
//multiple passes of singleMerge();

//return the resulting output stream

//input streams are assumed to be sorted, and are not necessarily of
//the same length.

//The comparison object "cmp", of (user-defined) class represented by
//Compare, must have a member function called "compare" which is used
//for sorting the input stream.  


template<class T, class Compare>
AMI_STREAM<T>* 
multiMerge(queue<char*>* runList, Compare *cmp) {
  AMI_STREAM<T> * mergedStr= NULL;
  char* path;
  
  assert(runList && runList->length() > 1  && cmp);

  SDEBUG cout << "multiMerge: " << runList->length() << " runs"  << endl;
  
  while (runList->length() > 1) {
    
    //merge streams from streamlist into mergedStr
    mergedStr = singleMerge<T,Compare>(runList, cmp);
    //i thought the templates are not needed in the call, but seems to
    //help the compiler..laura
    assert(mergedStr);
    
    //if more runs exist, delete this stream and add it to list
    if (runList->length() > 0) {
      mergedStr->name(&path);    
      runList->enqueue(path);
      mergedStr->persist(PERSIST_PERSISTENT);
      delete mergedStr;
    }
  }

  assert(runList->length() == 0);
  assert(mergedStr);
  return mergedStr;
}




#endif
  
