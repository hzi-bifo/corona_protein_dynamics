library(ggplot2)
library(tidyverse)

args = commandArgs(trailingOnly=TRUE)

data <- data.matrix(read.csv2(args[1], stringsAsFactors = FALSE, header = TRUE, row.names = 1, sep = "\t"))
data <- data[, colSums(data != 0) > 0]
data <- data[rowSums(data[, -1])>0, ]
rown <- row.names(data)
coln <- colnames(data)

df <- data.frame(matrix(ncol=length(coln),nrow=0, dimnames=list(NULL, coln)))
df2 <- data.frame(matrix(ncol=length(coln),nrow=0, dimnames=list(NULL, coln)))

l <- c()
n <- c()
m <- c()

#split up data into sign and non-sign
for (row in rown){
	pos <- strsplit(row,"_")[[1]][1]	#get substituttion
	#pos <- gsub("[^0-9.-]", "", pos)	#get position
	if (grepl("sign", row, fixed = TRUE)){
		df <- rbind(df,data[row,])
		data <- data[! (row.names(data) %in% row) , ]
		row <- paste(pos , "_sign")
		n <- c(n,row)
	} else {
		df2 <- rbind(df2, data[row,] )
		l <- c(l,pos)
		m <- c(m,row)
	}

}

rownames(df2) <- m
colnames(df2) <- coln
data <- df2
colnames(df) <- coln
rownames(df) <- n

rown <- row.names(data)
l <- unique(l)
r <- c()
for (pos in l){
	k <- c()
	for (row in rown){
		p <- strsplit(row,"_")[[1]][1]	#get substituttion
		#pos <- gsub("[^0-9.-]", "", pos)	#get position
		if (p == pos){
			k <- c(k,row) 
		}
	}
	if (length(k) == 1){

		df <- rbind(df,data[k,])
	
	} else {
		df <- rbind(df,colSums(data[k,]))
	}
	
}
colnames(df) <- coln
rown <- c(n,l)
rownames(df) <- rown

lineages <- colnames(df)
mutations <- row.names(df)
data_ <- df %>%
  rownames_to_column() %>%
  gather(lineages, value, -rowname)

values <- as.vector(t(df))
newdata <- expand.grid(X=rev(lineages),Y=rev(mutations))
newdata$Z <- rev(values)

#png(width=4000, height=4000,res=300)
pdf(width=25, height=90)

ggplot(newdata, aes(X,Y)) + 
geom_tile( aes(fill = Z), colour = "black") + 
scale_fill_gradientn(colours = c("white","blue", "red"), values = c(0, min( newdata$Z[newdata$Z!=min(newdata$Z)] ),1)) + 
#scale_fill_gradientn(colours = c("white","blue", "red"), values = c(0, min(newdata$Z) ,1)) + 
#ggtitle("Pangolin lineages and SD plot substitutions") +
labs(y="Amino acid changes", x = "Pangolin lineages") +
theme(axis.text.x = element_text(angle = 90)) 
