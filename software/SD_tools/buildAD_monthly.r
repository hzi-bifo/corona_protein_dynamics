#!/usr/bin/env Rscript 
args <- commandArgs(trailingOnly = TRUE)
set.seed(28123)


# read parameters
min_year <- as.numeric(args[1])
max_year <- as.numeric(args[2])
thresh_color <- as.numeric(args[4])



source(paste(args[5],"buildAD_fun_monthly.R", sep=""))
buildAD_monthly(infile.prefix = args[3], cutoff = 0, thresh_color=thresh_color, 
        minyear=min_year, maxyear=max_year, drucke=TRUE, startingSeason=args[6] )
