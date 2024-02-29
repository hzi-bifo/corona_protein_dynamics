#!/usr/bin/env Rscript

args <- commandArgs(trailingOnly = TRUE)

name <- args[1]

# output of corrected run
significance <- paste(name, ".significance.txt", sep = "")
d.significance <- read.table(significance, sep="\t", stringsAsFactors=FALSE)
mutations <- paste(name, ".mutations.per_position.txt", sep = "")
d.mutations <- read.table(mutations, sep="\t", stringsAsFactors=FALSE)

# get significant mutations
results <- d.mutations[which(rowSums(d.significance) > 0)]

# output of uncorrected run
mutations_nocor <- paste(name, ".mutations.per_position.txt", sep = "")
d.mutations_nocor <- read.table(mutations_nocor, sep="\t", stringsAsFactors=FALSE)
mutmap_nocor <- paste(name, ".subtreeMutationMap.per_position.txt", sep = "")
d.data_nocor <- read.table(mutmap_nocor, sep="\t", stringsAsFactors=FALSE)

if (!all(d.mutations == d.mutations_nocor)){
  print("Error: Mutation list in corrected run differs from uncorrected run. Should be deterministic. Check input.")
}

# combine information of mutation names, significance and number of leaves
results <- cbind(t(d.mutations_nocor), rowSums(d.significance, na.rm = TRUE), rowSums(d.data_nocor, na.rm = TRUE))
write.table(results, file=paste(name, ".summed_isolates.txt", sep = ""), sep = "\t", row.names = FALSE, col.names = FALSE, quote = FALSE)

# get only significant results
results_onlysign <- results[results[,2] == 1,,drop=FALSE]

# get season in which mutation was significant to distinguish equal mutations
season <- unlist(apply(d.significance, 1, function(x){which(unlist(x) == 1)}))
results_onlysign <- cbind(results_onlysign[,1], results_onlysign[,3], season)

write.table(results_onlysign, file=paste(name, ".summed_isolates.results.txt", sep = ""), sep = "\t", row.names = FALSE, col.names = FALSE, quote = FALSE)
