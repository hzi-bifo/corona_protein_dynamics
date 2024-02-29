#!/usr/bin/env Rscript 
args <- commandArgs(trailingOnly = TRUE)
set.seed(28123)

# read parameters
thresh_color <- as.numeric(args[2])
width <- as.numeric(args[5])
support <- as.numeric(args[7])

source(paste(args[3],"/buildAD_sign_fun_monthly.r", sep=""))
buildAD_monthly(infile.prefix = args[1], cutoff = 0, thresh_color=thresh_color, 
        drucke=TRUE, filter_sign=args[4], plotwidth=width, format=args[6], support_cutoff=support)
