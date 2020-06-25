# Information-retrieval-vector-space-model
## Compile
```
make
```
## Usage
```
./Main -d [Corpus_Path] -m [Model_Path] -k [Topk_tfidf_term_of_doc]
# You can find output inverted_file under topk_term_inverted_file/ directory
# You can find the total number of term selection on the last line of standard output
# You can also edit my_topk_exec.sh to execute 
```

## Example
```
./Main -d ~/IR-Kaggle/CIRB010 -m ~/IR-Kaggle/model -k 100
# Execution Time 55s
# Number of term selection: 1176688
```
