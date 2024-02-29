#!/usr/bin/env Rscript

library("ape")

args = commandArgs(trailingOnly=TRUE)
tree_file <- args[1]
outgroup <- args[2]

tree <- read.tree(tree_file)
tree_rooted <- root.phylo(tree, outgroup)

#replace nan edge lengths with 0
#tree_rooted$edge.length[is.nan(tree_rooted$edge.length)] <- 0

#overwrite tree
write.tree(tree_rooted, tree_file)
