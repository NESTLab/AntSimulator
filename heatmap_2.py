import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
import glob
import os

path="/home/ashay/Downloads/basic_attack_xp/replication_heatmap_focus/"
# path="/home/khaiyichin/research/hacking_the_colony_data/sim_run_112822/deep_dive_evap_rate/alpha/"
evapor_rate_range = [0.0, 0.5, 1.0, 2.0, 5.0, 10.0, 50.0, 100.0, 500.0, 1000.0]
# evapor_rate_range = list(range(1,16,2))
detractor_percentage_range = 2.0 ** -np.arange(1,11)
# detractor_percentage_range = [0.03125]
evapor_rate_range.reverse()

spec_data = np.zeros((len(evapor_rate_range),len(detractor_percentage_range),4))

# Extract and average data
for i, e in enumerate(evapor_rate_range):
    for j, m in enumerate(detractor_percentage_range):
        files = glob.glob(path + "evap_rate_" + str(e) + "_frac_" + str(m) + "/*.csv")
        # files = glob.glob(path + "evap_rate_" + str(e) + "/*.csv")
        for f in files:
            if os.path.splitext(f)[1] == ".csv":
                dt = np.array(pd.read_csv(f, header=None))
                modified_data = dt[-1]
                modified_data = (modified_data)/(1-m)
                spec_data[len(evapor_rate_range)-i-1,len(detractor_percentage_range)-j-1] += modified_data/len(files)

# Define heatmap axis labels
x = np.array(evapor_rate_range)
y = np.array(detractor_percentage_range)
x=x[::-1]
y=["{0:.2f}".format(i*1e2) for i in y[::-1]]

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
        df[i] = df[i].rename_axis("Fake Pher. Evap. Rate")
    if(i>1):
        df[i] = df[i].rename_axis("Malicious Ants Percentage", axis=1)

    # Set heatmap title
    if (i == 0): ax.set_title("Average food bits per ant\n(collected)")
    elif (i == 1): ax.set_title("Average food bits per ant\n(delivered)")
    elif (i == 2): ax.set_title("Average cooperator fraction\n(collected)")
    else: ax.set_title("Average cooperator fraction\n(delivered)")
    
    plot = sns.heatmap(df[i], ax=ax, xticklabels=y, yticklabels=x, cmap=color_palette, cbar=True, annot=True, fmt=".2f", vmin=0, vmax=1 if i>1 else 22, annot_kws={"fontsize":5})
    ax.set_xticklabels(y, rotation=45, va="top", ha="center")
    ax.set_yticklabels(x, rotation=0)

plt.subplots_adjust(hspace=0.5)
plot.figure.savefig(os.path.join(path, "replication_heatmap_focus.png"), dpi=400)