args<-commandArgs(TRUE)

mydata <- read.table(args[1], header=TRUE, sep="\t")[-c(1),]
library(ggplot2)

m <- mean(mydata$num)
name <- gsub("_covid_complete","",gsub("seq_counts_","",gsub(".csv",".pdf", args[1])))

l <- length(mydata[,1])
size <- if (l > 10) l else 10

pdf(gsub(".pdf",".seq_stats.pdf",name),width=length(mydata[,1]))

ggplot(data=mydata, aes(x=date, y=num, width=.5)) + 
	ggtitle(gsub(".pdf","",name)) + 
		theme_minimal() + 
			theme(plot.title = element_text(size=14, face="bold" , hjust = 0.5)) +
				geom_hline(yintercept=m, linetype="dashed", color = "red") +
					geom_bar(stat="identity", fill="steelblue") +
						geom_text(aes(label=num), vjust=-0.3, size=3.5)
