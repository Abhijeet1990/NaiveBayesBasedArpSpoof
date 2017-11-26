# follow.py
#
# Follow a file like tail -f.
import matplotlib.pyplot as plt
import numpy as np
from sklearn.naive_bayes import BernoulliNB
import time


def follow(thefile,thefile4):
    thefile.seek(0,1)
    thefile4.seek(0,1)
    while True:
        line = thefile.readline()
        line4 = thefile4.readline()
        
        if not line or not line4:
            time.sleep(0.1)
            continue
        
        yield line, line4

def LowerBound(x):
    mean=np.mean(x)
    deviation=np.std(x)
    return mean - deviation


def UpperBound(x):
    print "x size",len(x)
    mean=np.mean(x)
    deviation=np.std(x)
    return mean + deviation

#To reduce Mathematical complexity we assume the Plower = 0.01 and Phigher = 0.99
#calculate the Probability of Likelihood of observing the feature when you a specific label Normal or Abnormal
# P(Feature1,Feature2,...FeatureN/Abnormal) or P(Feature1,Feature2,....FEatureN/Normal)
def Likelihood(Prtt, Pgrat):
    return np.log(Prtt)+np.log(Pgrat)

#Upgrade the Conditional Prob of a given Feature given the Label is Normal
# P(Feature/Normal) or P(Feature/Abnormal)
def Feature_Prob(Feature):
    count=0
    s=0
    for i in Feature:
        s = s+i;
        count=count+1
    #print "conditional prob",np.abs(s)/count
    return np.abs(s)/count

#Upgrade the Pthreshold value..if it is not upgraded it has a scope of misjudgement and probability of ommision
#alpha: it is the average of the Normal/Abnormal Label 
#A: it is the average of Normal/Abnormal Label for last 10 packets
#If A < alpha , we need to improve the Pthres to reduce the misjudgement...because if A has less Abnormal Label 
#weight: how much we give weight to the last 5 packets information
#If we observe a large chunk of Abnormal Labels than we raise the Pthres to avoid large False Positives in the detection
def Upgrade_PThres(alpha,A,PThres):
    weight=0.1
    Pmin=0.4
    decision_factor = PThres + (alpha - A)*weight
    if decision_factor>=1:
       PThres=1
    elif decision_factor>=Pmin and decision_factor<=1.0:
       PThres=decision_factor
    else:
       PThres=Pmin
    return PThres 
    
def CalcPosterior(Prior,Likelihood_Normal,Likelihood_Abnormal):
    P_Attack = Prior
    P_NonAttack = 1 - Prior
    #x = np.log(P_Attack)+np.log(Likelihood_Abnormal)
    #y = np.log(P_NonAttack)+np.log(Likelihood_Normal)
    x=P_Attack*np.exp(Likelihood_Abnormal)
    y=P_NonAttack*np.exp(Likelihood_Normal)
    Posterior= x/(x+y)
    return Posterior
    
def DetectAttack(Pthres,Posterior):
    print "xxxxxxxxxPthres =",Pthres
    print "xxxxxxxxxPosterior = ",Posterior
    if(Posterior > Pthres):
       print("Attack detected")
       return 0.99
    else:
       print("No Attack detected")
       return 0.01
    
       

if __name__ == '__main__':
    B = 10  
    #B is the block size
    log_train1 = open("rtt_train_file","r")
    log_train4 = open("grat_train_file","r")
    
    #Train_Len=len(log_train1)
    #we consider here 4 features...
    #1)Round Trip Time
    #2)Gratuitous ARP reply

    #building a model using the trained data 
    #Define the prior probability of a packet being an attack packet as 0.01
    #upgrade the Prior after every iteration from the Posterior
    Prior = 0.01 
    #PThres = Prior/2
    PThres = 0.5

    

    #Training matrix consist of Label, RTT, RSS, Seq_Flag, Grat
    trainingMat = np.zeros((B,3),dtype='float')
    testingMat = np.zeros((B,3),dtype='float')
    store_rtt = []
    store_grat = []
    store_Label = []
    store_rtt_A = []
    store_grat_A = []
    store_rtt_N = []
    store_grat_N = []
    #t=[]
    #plotrtt=[]
    i=0
    s=0
    g=1
    Normal = 0.99
    Abnormal = -0.99
    attack_count=0
    for k,n in follow(log_train1,log_train4):
        rtt=float(k)
        plotrtt.append(rtt)
        grat=float(n)
        print("{0}\t{1}\t".format(rtt,  grat))
        
        #Observe the average RTT and RSS for 20 packets...split the training and Testing Data into 50:50 ratio
        if (i<= B ):
           t.append(g)
           store_rtt.append(rtt)
           store_grat.append(grat)
           
           if i < B: #for the first 10 packets train the model 
              rtt_lower_bound = LowerBound(store_rtt)
              rtt_upper_bound = UpperBound(store_rtt)
              print "Upper Bound RTT",rtt_upper_bound
              #print "Lower Bound RTT",rtt_lower_bound
             
              #Map the Features into Normal and Abnormal Based on the values
              #For the Round Trip time
              #if (rtt > rtt_lower_bound and rtt < rtt_upper_bound):
              if (rtt <= rtt_upper_bound):
           	trainingMat[i][1] = Normal
              else:
                trainingMat[i][1] = Abnormal
             
              #For Gratituous Detection
              if grat == 0.0 :
                trainingMat[i][2] = Normal
              else:
                trainingMat[i][2] = Abnormal
          
              #upgrade the Label 
              if (trainingMat[i][1] == Abnormal) or (trainingMat[i][2] == Abnormal) :
                trainingMat[i][0] = Abnormal
              else:
                trainingMat[i][0] = Normal
              if trainingMat[i][0]>0:
                  store_Label.append(0.01)
              else:
                  store_Label.append(0.99)

              #Now Based on the Label Split the storage of the normalized Feature for Likelihood Calculation
#if it is an abnormal tagged then normalize the data to 0.01 if the feature is normal and normalize the data to 0.99 if the feature is abnormal
              if (trainingMat[i][0] == Abnormal):
                  if trainingMat[i][1]>0:#if feature is normal
                      store_rtt_A.append(0.01)
                  else:#if feature is abnormal
                      store_rtt_A.append(0.99)
                  if trainingMat[i][2]>0:#if feature is normal
                      store_grat_A.append(0.01)
                  else:#if feature is abnormal
                      store_grat_A.append(0.99)
                  
              if (trainingMat[i][0] == Normal):
                  if trainingMat[i][1]>0:
                      store_rtt_N.append(0.01)
                  else:
                      store_rtt_N.append(0.99)
                  if trainingMat[i][2]>0:
                      store_grat_N.append(0.01)
                  else:
                      store_grat_N.append(0.99)
                  
              i=i+1
              g=g+1
              Prtt_A=Feature_Prob(store_rtt_A)
              Prtt_N=Feature_Prob(store_rtt_N)
              Pgrat_A=Feature_Prob(store_grat_A)
              Pgrat_N=Feature_Prob(store_grat_N)
                  
              #Upgrade the Feature Likelihood of Non Attacker and Attacker Packet
              Likelihood_Normal = Likelihood(Prtt_N,Pgrat_N)
              print "Likelihood_Normal", Likelihood_Normal
              Likelihood_Abnormal = Likelihood(Prtt_A,Pgrat_A)
              print "Likelihood_Abnormal", Likelihood_Abnormal
              #Upgrade the Posterior Probability of a packet being an attack packet
              Posterior_A = CalcPosterior(Prior, Likelihood_Normal, Likelihood_Abnormal)
              print "Posterior Abnormal",Posterior_A
              #Upgrade the Posterior Probability of a packet being not an attack packet
              Posterior_N = CalcPosterior(1-Prior, Likelihood_Abnormal, Likelihood_Normal)
              print "Posterior Normal" ,Posterior_N

              #when we reach the end of the training block we calculate the Likelihoods
           if(i == B):
                  
                  print store_Label
                  print store_rtt_A
                  print store_rtt_N
                  print store_grat_A
                  print store_grat_N
                  
                  Prtt_A=Feature_Prob(store_rtt_A)
                  Prtt_N=Feature_Prob(store_rtt_N)
                  Pgrat_A=Feature_Prob(store_grat_A)
                  Pgrat_N=Feature_Prob(store_grat_N)
                  
                  #Upgrade the Feature Likelihood of Non Attacker and Attacker Packet
                  Likelihood_Normal = Likelihood(Prtt_N,Pgrat_N)
                  print "Likelihood_Normal", Likelihood_Normal
                  Likelihood_Abnormal = Likelihood(Prtt_A,Pgrat_A)
                  print "Likelihood_Abnormal", Likelihood_Abnormal
                  #Upgrade the Posterior Probability of a packet being an attack packet
                  Posterior_A = CalcPosterior(Prior, Likelihood_Normal, Likelihood_Abnormal)
                  print "Posterior Abnormal",Posterior_A
                  #Upgrade the Posterior Probability of a packet being not an attack packet
                  Posterior_N = CalcPosterior(1-Prior, Likelihood_Abnormal, Likelihood_Normal)
                  print "Posterior Normal" ,Posterior_N
                  #Upgrade the Pthreshold for the Attack detection based on alpha and A
                  #A: is the count of the number of Labels with Abnormals in the current 10 packets
                  #alpha: is the Average count of the number of Labels with Abnormals
                  
                  
                  
                  #i=i+1
                  NewPrior = Posterior_A
                  print "New Prior",NewPrior
                  #attack_count = attack_count+ DetectAttack(PThres,NewPrior)
                  A=DetectAttack(PThres,NewPrior)
                  #A = np.mean(trainingMat[[i-10,i-9,i-8,i-7,i-6,i-5,i-4,i-3,i-2,i-1],0])
                  #alpha = np.mean(store_Label)
                  
                  f=0
                  for z in range(1,2):
                     f=f+np.mean(trainingMat[[i-10,i-9,i-8,i-7,i-6,i-5,i-4,i-3,i-2,i-1],z])
                  alpha=f/2
                  
                  #alpha=np.mean(store_Label)
                  alpha=(np.mean(store_rtt_A)+np.mean(store_grat_A))/2
                  print("{0}\t{1}".format(A,alpha))
                  PThres = Upgrade_PThres(alpha,A,PThres)
                  print "Upgraded P Threshold",PThres
                  print "size of store_rtt",len(store_rtt)
                  print "size of g",len(t)
                  plt.plot(plotrtt,t)
                  
                  store_rtt=[]
                  store_grat=[]
                  store_Label = []
    	          store_rtt_A = []
                  store_grat_A = []
                  store_rtt_N = []
                  store_grat_N = []
                  i=0
             
 
                 
           #when the packet i > 10, we need to check the feature of the test data to see if  
           if (i > B/2):
             
             LLA=0
             LLN=0
             #For RTT 
             if (rtt < rtt_upper_bound):
           	testingMat[i-11][1] = Normal
             else:
                testingMat[i-11][1] = Abnormal
           
             #For Gratituous Detection
             if grat == 0 :
                testingMat[i-11][2] = Normal
             else:
                testingMat[i-11][2] = Abnormal
             
           
             if testingMat[i-11][1] == Abnormal:
               LLA = LLA + np.log(Prtt_A)
             else:
               LLN = LLN + np.log(Prtt_N)
             
             if testingMat[i-11][2] == Abnormal:
               LLA = LLA + np.log(Pgrat_A)
             else:
               LLN = LLN + np.log(Pgrat_N)

             print "LLN",LLN
             print "LLA",LLA
             #Upgrade the Posterior Probability of a packet being an attack packet
             Test_Post_A = CalcPosterior(NewPrior, LLN, LLA)
             print "Test Posterior Attack",Test_Post_A
              
             #Upgrade the Posterior Probability of a packet being not an attack packet
             Test_Post_N = CalcPosterior(1-NewPrior, LLA, LLN)
             print "Test Posterior No attack", Test_Post_N
             attack_count = attack_count+ DetectAttack(PThres,Test_Post_A)
             
             i=i+1
             
           if i == 20:
             store_rtt=[]
             store_grat=[]
             i=0
             
          
    #We will give the training data to fit into the Naive Bayes Model
    
    ber = BernoulliNB()
    ber.fit(Train_Features, Train_Labels)

    testMat = np.array
    testMat = np.zeros((Train_len,5),dtype='i,f,f,i,b')
    for rtt,rss,seq_flag,grat in zip(logtest_rtt,logtest_rss,logtest_seq,logtest_grat):
         testMat[j,1]=
         testMat[j,0]= ber.predict(testMat[j,:])  
         j=j+1
        
    


    
    
