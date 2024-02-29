#!/usr/bin/env Rscript
buildAD_monthly <- function(infile.prefix, cutoff, thresh_color, minyear,
                    maxyear, drucke, startingSeason){
    
    mutations <- paste(infile.prefix, ".mutations.txt", sep="")
    mutmap <- paste(infile.prefix, ".subtreeMutationMap.txt", sep="")
    numiso <- paste(infile.prefix, ".numIsolates.txt", sep="")
    filename <- paste(infile.prefix, ".pdf", sep="")
    sumyfile <- paste(infile.prefix, "summary.txt", sep="")
    month <- paste(infile.prefix,"_time_map.csv", sep="")
    
    if(drucke==TRUE){
        pdf(file=filename, width = 12, height = 8)
    }

  	d.names <- read.table(mutations, sep="\t", stringsAsFactors=FALSE)
  	d.data <- read.table(mutmap, sep="\t", stringsAsFactors=FALSE)
  	d.isolates <- read.table(numiso, sep=" ", stringsAsFactors=FALSE)
  	d.isolates <- as.numeric(d.isolates)

    d.data <- d.data[, -ncol(d.data)]
    d.years <- 1:ncol(d.data)

    months <- c(sapply(read.table(month, header=FALSE, sep="\t")[1],levels))
    print(months)
       
    tmp_years <- seq(minyear, maxyear+1, 0.5)

    # define the plot window
    if(startingSeason == "S"){
        plotYears <- ceiling(tmp_years)
        plotYears[tmp_years %% 1 == 0] <- paste(plotYears[tmp_years %% 1 == 0], "S",
                                            sep="")
        plotYears[tmp_years %% 1 > 0] <- paste(plotYears[tmp_years %% 1 > 0], "N",
                                           sep="")
    }else{
        plotYears <- floor(tmp_years)
        plotYears[tmp_years %% 1 == 0] <- paste(plotYears[tmp_years %% 1 == 0], "N",
                                            sep="")
        plotYears[tmp_years %% 1 > 0] <- paste(plotYears[tmp_years %% 1 > 0], "S",
                                           sep="")
    }
    
    plotYears <- plotYears[1:length(d.years)]
    print("read data, now setting names...")
    
#----------------------------------------------------------------------------#
# set names, for AD-Plots with longer sequences
    interNames <- rep("", ncol(d.names))
    for (i in 1:ncol(d.names)){
        # split main mutations from parent mutations      
      	tmpWholeSplit <- strsplit(d.names[1, i], "\\{")[[1]] 
      	tmpSplit <- strsplit(tmpWholeSplit[1], " ")[[1]]
      	
      	# to remove ancestral state, set start=2
      	for (j in tmpSplit){ 
            interNames[i] <- paste(interNames[i], substring(j, 1, nchar(j)), sep=" ")
      	}
      	
      	interNames[i] <- substring(interNames[i], 2, nchar(interNames[i]))
      	if (length(tmpWholeSplit) > 1){
            tmpSplit <- strsplit(substring(tmpWholeSplit[2], 1,
                                 nchar(tmpWholeSplit[2]) - 1), " ")[[1]]
            tmpNames <- ""
            for (j in tmpSplit) {
                tmpNames <- paste(tmpNames, substring(j, 2, nchar(j)), sep=", ")
            }
            if (tmpNames != ""){
                tmpNames <- substring(tmpNames, 3, nchar(tmpNames))
                interNames[i] <- paste(interNames[i], " *", tmpNames, "*", sep="")
            }
      	}
    }

    used <- rep(FALSE, ncol(d.names))
    d.names.old <- sapply(d.names[1, ], function(x) substr(x, 2, nchar(x)))

    print ("names set, now defining important alleles...")
    
#------------decide, which curves should be highlighted----------------------#
    test.data <- d.data
    norm.data <-round(d.data[, 1:length(d.isolates)] / 
              matrix(d.isolates, nrow=nrow(d.data), ncol=length(d.isolates), byrow=TRUE), 4)
    test.data <- norm.data[, d.isolates > 0]
    d.years <- d.years[d.isolates > 0]
    plotYears <- plotYears[d.isolates > 0]

    seli <- rep(FALSE, nrow(test.data)) # select important alleles
    for (i in 1:nrow(test.data)){
	      if (sum(test.data[i, ] > thresh_color) == 0) seli[i] <- TRUE
    }

    print ("important ones set, now building the plot...")
    
#--------basic framework for the allele dynamics plot------------------------#
    colVec <- rep("darkgray", nrow(test.data))
    colVec [!seli] <- sample(rainbow(sum(!seli)))
    ltyVec <- rep(1, nrow(test.data))
    lwdVec <- rep(1, nrow(test.data))
    lwdVec [!seli] <- 1.5
    pchVec <- rep(1, nrow(test.data))

    m <- ncol(test.data)
    n <- nrow(test.data)
    plot(d.years[d.years > cutoff], test.data[1, d.years > cutoff], type="n",
         col=colVec[1], ylim=c(0, 1), axes=FALSE, xlab="Month", ylab="Frequency",
         lwd=lwdVec[1])
    #axis(1,at=d.years[d.years>cutoff],labels=plotYears[d.years>cutoff],cex.axis=1.0)
    axis(1,at=d.years[d.years>cutoff],labels=months,cex.axis=1.0)
    axis(2, seq(0, 1, 0.1))
    # plot dotlines vertically at each season and two horizontally at 0 and 1
    segments((cutoff + 1), 0.0, max(d.years), 0.0, col="gray", lwd=1, lty=3)
    segments((cutoff + 1), 1.0, max(d.years), 1.0, col="gray", lwd=1, lty=3)
    for (i in d.years[d.years > cutoff]){
        segments(i, 0.0, i, 1.0, col="gray", lwd=1, lty=3)
    }
    
    for (i in (1:n)) {
	      tmpData <- test.data[i, d.years>cutoff]
        lines((d.years[d.years > cutoff]), tmpData, lwd=lwdVec[i], col=colVec[i],
        type="b", lty=ltyVec[i], pch=pchVec[i]) 
    }
#print("debug 11")

# ncol depends on the number of years that i want to plot!!!
    blocked <- matrix(FALSE, ncol=sum(d.years > cutoff)+1, nrow=31)
   # plotNames <- gsub("[A-Z]", "", interNames, ignore.case = TRUE, perl = TRUE)
    plotNames <- interNames
    for (i in (1:n)[colVec!="darkgray"]) {
	tmpData <- test.data[i, d.years > cutoff]
	sel <- which(tmpData < 1 & tmpData > 0.0)
	if (length(sel) == 0) next
	isSet <- FALSE
	for (j in sel) {
            tmpVal <- ((tmpData[j] - (tmpData[j] %% (1/30))) / (1/30))[1, 1]
            if ((tmpData[j] %% (1/30)) > 1/60) tmpVal <- tmpVal + 1
            if (tmpVal < 2) next
            if (sum(blocked[c(tmpVal - 1, tmpVal, tmpVal + 1), j + 1]) == 0) {
                blocked[c(tmpVal - 1, tmpVal, tmpVal + 1), j + 1] <- TRUE
                text((d.years[d.years > cutoff])[j] + 0.5, tmpData[j],
                     labels=plotNames[i], cex=1, adj=0, font=2) # cex=1.2, font=4)
                segments((d.years[d.years > cutoff])[j] + 0.2, tmpData[1, j],
                         (d.years[d.years > cutoff])[j] + 0.4, tmpData[1, j],
                         col=colVec[i], lwd=2)
                isSet <- TRUE
                break
            }
	}
	if (isSet == FALSE) {
            print (paste(plotNames[i], " is missing", sep=""))
            print (sel)
            print (tmpData[sel])
	}
    }
#print("debug 22")

    resList <- vector("list", ncol(test.data) - 1)
    for (i in 2:ncol(test.data)) {
        sel <- test.data[, i] > 0.05 & test.data[, i] < 9.5
        resList [[i - 1]] <- cbind(interNames[sel], test.data[sel, i],
                                   test.data[sel, i] - test.data[sel, i - 1])
    }
    
    
    names(resList) <- plotYears[2:(length(plotYears))]
    
#print("debug 33")

    for (i in 1:length(resList)){
        cat(names(resList)[i], "\n", file=sumyfile, append=T, sep="")
        write.table(resList[[i]], sumyfile, row.names=FALSE, col.names=F, sep="\t",
                    quote=FALSE, append=TRUE)
    }

    if(drucke==TRUE){
        dev.off()
    }
}
