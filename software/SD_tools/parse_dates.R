#!/usr/bin/env Rscript

library("jsonlite")

args = commandArgs(trailingOnly=TRUE)
file <- args[1]
output <- args[2]

# read header of all fasta sequences
header <- read.table(file, stringsAsFactors = FALSE)

# split to get epi id and date
header_split <- strsplit(header$V1, "|", fixed = TRUE)
epi_id <- trimws(unlist(lapply(header_split, function(x) x[[2]])))
date <- trimws(unlist(lapply(header_split, function(x) x[[3]])))

epi_number <- unlist(lapply(strsplit(epi_id, "_", fixed = TRUE), function(x) x[[3]]))
date <- substr(date, 1, 7) # only year and month

date_unique <- unique(date)
date_list <- list()

# go through all unique dates to get a list of all isolates in that time frame
for(current_date in date_unique){
  # add ids for current date to list
  date_list[[current_date]] <- as.integer(epi_number[date == current_date])
}

# save list as json file
date_json <- toJSON(date_list, pretty = TRUE)
write(date_json, output)
