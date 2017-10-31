rm(list = ls())
setwd("C:/Users/nickc/Box Sync/2016Q4/MS&E226 Small Data/Problem Sets/PS4")
set.seed(123)

df = read.csv("spam.csv")

train.ind = sample(1:nrow(df), 0.75*nrow(df))
df.train = df[train.ind,]
df.test = df[-train.ind,]

Q0 = nrow(subset(df.train, spam == 0)) / nrow(df.train)
Q1 = nrow(subset(df.train, spam == 1)) / nrow(df.train)
Q0
Q1

q0 = rep(NA, 48)
q1 = rep(NA, 48)

for (i in c(1:(ncol(df) - 1))){
  q0[i] = sum(subset(df.train, spam == 0)[ ,i]) / (nrow(df.train) - sum(df.train$spam))
  q1[i] = sum(subset(df.train, spam == 1)[ ,i]) / sum(df.train$spam)
}
q0
q1

spamNB = function(x, thresh = 0) {
  P1 = log(Q1) + sum( x*log(q1) + (1-x)*log(1 - q1) )
  P0 = log(Q0) + sum( x*log(q0) + (1-x)*log(1 - q0) )
  if (P1 - P0 > thresh)
    return(1)
  else
    return(0)
}

result = rep(NA, nrow(df.test))
for (i in 1:nrow(df.test)){
  result[i] = spamNB(df.test[i, 1: ncol(df.test) - 1])
}
error = sum(abs(result - df.test$spam)) / nrow(df.test)
error

result_thres = rep(NA, nrow(df.train))
error_thres = rep(NA, 11)
lambdas = seq(-50, 50, by = 1)
index = 1
for (lambda in lambdas) {
  print (lambda)
  for (i in 1:nrow(df.train)){
    result_thres[i] = spamNB(df.train[i, 1: ncol(df.train) - 1], thresh = lambda)
  }
  error_thres[index] = sum(abs(result_thres - df.train$spam)) / nrow(df.train)
  index = index + 1
} 
error_thres
plot(lambdas, error_thres)
min(error_thres)
lambdas[which.min(error_thres)]