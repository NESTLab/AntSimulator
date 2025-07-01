import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
import glob

path = r'./data/evap_rate_1.0_frac_0.03125' # use your path
evapor_set = [0.0,0.5,1.0,2.0,5.0,10.0,50.0,100.0,500.0,1000.0]
evapor_set.reverse()
all_files = glob.glob(path + "/*_iter-0.csv")

spec_data = np.zeros((10,10,4))

for i, e in enumerate(evapor_set):
    for m in range(1,11):
        for file in all_files:
            if ("mal_frac-"+str(("%.6f" % (2**-m)))) in file:
                if ("evpr-"+str(e)) in file:
                    # print(m)
                    dt = np.array(pd.read_csv(file, header=None))
                    modified_data = dt[-1]
                    modified_data = (modified_data)/(1-(2**-m))
                    spec_data[9-i,9-(m-1)] = modified_data
                    break
    

# #heatmap axis
x = np.array(evapor_set)
# y = [str(n) for n in range(1,11)]
y = np.array(["50.00","25.00","12.50","6.25","3.13","1.56","0.78","0.39","0.20","0.10"])
x=x[::-1]
y=y[::-1]
color_blue = sns.color_palette("coolwarm_r", as_cmap=True)
color_green = sns.color_palette("coolwarm_r", as_cmap=True)

fig,axn = plt.subplots(2, 2, sharex=True, sharey=True)

df=[]

for i in range(spec_data.shape[-1]):
    df.append(pd.DataFrame(spec_data[:,:,i]))

for i,ax in enumerate(axn.flat):
    if(i%2==0):
        df[i] = df[i].rename_axis("Rel Evaporation Multi")
    if(i>1):
        df[i] = df[i].rename_axis("Malicious Ants Fraction", axis=1)
    color = color_blue if i<2 else color_green
    plot = sns.heatmap(df[i], ax=ax, xticklabels = y, yticklabels = x, cmap=color, cbar=not(i%2==0))
    plt.xticks(rotation = 45)

plot.figure.savefig("graphs/heatmap_all_data.png")
