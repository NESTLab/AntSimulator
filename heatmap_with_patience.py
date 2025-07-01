import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
import glob
import os

path="/home/ashay/Downloads/basic_attack_xp/replication_heatmap_focus"
patience_max = [50, 100, 250, 500, 750, 1000]
patience_refill_period = [1, 2, 5, 10, 50, 100, 1000]
patience_max.reverse()
patience_refill_period.reverse()
mal_frac = 2**(-5) # alpha
# mal_frac = 2**(-3) # delta

spec_data = np.zeros((len(patience_max),len(patience_refill_period),4))

# Extract and average data
for i, m in enumerate(patience_max):
    for j, r in enumerate(patience_refill_period):
        files = glob.glob(path + "prd_" + str(r) + "_max_" + str(m) + "/*.csv")
        for f in files:
            if os.path.splitext(f)[1] == ".csv":
                dt = np.array(pd.read_csv(f, header=None))
                modified_data = dt[-1]
                modified_data = (modified_data)/(1-mal_frac)
                spec_data[len(patience_max)-i-1,len(patience_refill_period)-j-1] += modified_data/len(files)

# Define heatmap axis labels
x = np.array(patience_max)
y = np.array(patience_refill_period)
x=x[::-1]
y=y[::-1]

# Rearrange data for heatmap plotting
df=[]

for i in range(spec_data.shape[-1]):
    df.append(pd.DataFrame(spec_data[:,:,i]))

# Plot heatmaps
fig,axn = plt.subplots(2, 2, sharex=True, sharey=True)
fig.set_tight_layout(True)
fig.set_size_inches((8.5,8))
color_palette = sns.color_palette("coolwarm_r", as_cmap=True)

for i,ax in enumerate(axn.flat):
    if(i%2==0):
        df[i] = df[i].rename_axis("Max. Patience Value")
    if(i>1):
        df[i] = df[i].rename_axis("Patience Refill Period", axis=1)

    # Set heatmap title
    if (i == 0): ax.set_title("Average food bits per ant\n(collected)")
    elif (i == 1): ax.set_title("Average food bits per ant\n(delivered)")
    elif (i == 2): ax.set_title("Average cooperator fraction\n(collected)")
    else: ax.set_title("Average cooperator fraction\n(delivered)")
    
    plot = sns.heatmap(df[i], ax=ax, xticklabels = y, yticklabels = x, cmap=color_palette, cbar=True, annot=True, fmt=".2f", vmin=0)#, vmax=1 if i>1 else 9.2)
    ax.set_xticklabels(y, rotation=45, va="top", ha="center")
    ax.set_yticklabels(x, rotation=0)

# exit()
plt.subplots_adjust(hspace=0.5)
plot.figure.savefig(os.path.join(path, "random_no_focus_patience_alpha.png"), dpi=400)