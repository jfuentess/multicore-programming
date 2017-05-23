#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <stdint.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#include "dictionary.h"
#include "../../LZscan/algorithm/lzscan.h"

using std::pair;
using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::pair;
using std::string;

// Parse the reference using a LZ77 compressor
size_t lz_parse_ref(uint8_t * seq,
                    size_t seq_len,
                    Writer &w,
                    int max_memory_MB) {
  vector<pair<int, int>> ref_parsing;
  int n_phrases = parse(seq, 
                        seq_len, 
                        max_memory_MB << 20, 
                        &ref_parsing);
  for (int i = 0; i < n_phrases; i++) {
    w.write(ref_parsing[i].first, 64);
    w.write(ref_parsing[i].second, 64);
  }
  return (size_t)n_phrases;
}

// Write the list of factor into the output
size_t flush_phrases(vector<vector<Factor>> factor_lists, Writer &w) {
  size_t n_factors = 0;
  w.flush();
  for (size_t t = 0; t < factor_lists.size(); t++) {
    n_factors += factor_lists[t].size(); 
     
    size_t data_len = factor_lists[t].size();
    if (data_len != fwrite(&(factor_lists[t][0]),
                           sizeof(Factor),
                           data_len, 
                           w.fp)) {
      std::cerr << "Error flushing phrases " << endl;
    }
  }
  return n_factors;
}

// Parse a partition of the text
template <typename sa_t>
void chunk_parse(uint8_t* buffer,
                 size_t length,
                 Dictionary<sa_t> &d,
                 vector<Factor> & ans) {
  size_t i = 0;
  while (i < length) {
    // Finding the longest matching substring in the reference
    Factor factor = d.at(buffer + i, length - i);
    ans.push_back(factor);
    i += (factor.len > 0) ? factor.len : 1;
  }
  return;
}

// Parse a partition of the text
template <typename sa_t>
size_t parse_ref(Dictionary<sa_t> &d,
                 Writer &w,
                 int max_memory_MB) {
  size_t n_factors; 

  size_t EMLZ_limit_MB = (29L*(INT32_MAX/2))/(1L<<20);
  size_t EMLZ_mem_MB = std::min(EMLZ_limit_MB, (size_t)max_memory_MB);

  n_factors = lz_parse_ref(d.d, d.n, w, EMLZ_mem_MB);

  return n_factors;
}

void read_block(uint8_t *buffer, size_t buffer_len, size_t block_id, FILE * fp) {
  if (buffer_len != fread(buffer, 1, buffer_len, fp)) {
    printf("Error reading block %ld\n", block_id);
    exit(EXIT_FAILURE);
  }
}

// Parse a block/bugger of the input text
template <typename sa_t>
size_t process_block(uint8_t *buffer, size_t buffer_len, size_t block_offset,
		     size_t block_id,  FILE * fp, size_t n_partitions,
		     Dictionary<sa_t>& d, Writer& w) {

  // Reading the buffer
  read_block(buffer, buffer_len, block_id, fp);

  // Creating a list of vectors to store the factors of each partition
  vector<vector<Factor>> factor_lists(n_partitions);

  // Processing the current buffer partition by partition
  for (size_t t = 0; t < n_partitions; t++) {
    size_t starting_pos = t*(buffer_len/n_partitions);
    size_t length = (t != n_partitions - 1) ? (buffer_len/n_partitions) :
      buffer_len - starting_pos; 
    
    assert(d.n <= block_offset + starting_pos);

    // Processing the partition t
    chunk_parse(buffer+starting_pos, length, d, factor_lists[t]); 
  }

  // Writing the list of factor into the output
  size_t n_factors = flush_phrases(factor_lists, w);
  return n_factors;
}

// Principal function to process a text using an external memory RLZ algorithm
template <typename sa_t>
size_t parse_in_external_memory(char * input_filename,
                                char * reference_filename, 
                                size_t n_partitions,
                                int max_memory_MB,
                                Writer &w) {

  // Creating the dictionary, based on the reference
  Dictionary<sa_t> d(reference_filename, INT32_MAX);
  d.BuildSA();

  // Parsing the reference
  size_t n_factors = 0;
  n_factors = parse_ref<sa_t>(d, w, max_memory_MB);  

  size_t text_len;
  FILE * fp = open_file(input_filename, &text_len);

  // Size of each buffer: The size corresponds to the maximum allowed memory
  // minus the size of the dictionary
  size_t block_len = ((size_t)max_memory_MB << (size_t)20) - d.size_in_bytes();
  
  if (block_len > text_len)
    block_len = text_len;
  
  // We will read blocks of size "block_len" bytes
  size_t block_offset = d.n;
  fseek(fp, block_offset, SEEK_SET);
  size_t block_id = 0;
  uint8_t *buffer = new uint8_t[block_len + 1];

  // Processing block by block
  while (block_offset < text_len) {
    size_t buffer_len = (block_offset + block_len < text_len) ? block_len :
      text_len - block_offset;

    // Each block is divided in partitions. The total number of partitions is
    // given in n_partitions 
    n_factors += process_block(buffer, buffer_len, block_offset, block_id, fp,
			       n_partitions, d, w);
    
    block_offset += buffer_len;
    block_id++;
  }
  
  delete [] buffer;
  fclose(fp);
  fflush(stdout);
  
  return n_factors;
}
