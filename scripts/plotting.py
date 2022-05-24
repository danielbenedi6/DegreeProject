import pandas
import statistics
import matplotlib.pyplot as plt
import math
from scipy.stats import linregress
from numpy.lib.scimath import log

name={
      "s": "Serial canonical splitting",
      "p0": "Parallel canonical splitting",
      "p1": "Parallel surface area heuristic",
      "p2": "Parallel curve complexity heuristic",
      "p3": "Parallel median of hyperplane with max variance",
      "p4": "Parallel minimum variance union"
      }

def plot(data, iterable):
    fig, (ax_b, ax_q, ax_h) = plt.subplots(1,3)
    for exec in data["type"].unique():
        X = data[data["type"] == exec][iterable].unique()
        X.sort()
        mean_building = []
        mean_search = []
        for x in X:
            t_building = data[ (data["type"] == exec) & (data[iterable] == x) & (data["hits"] != -1)]["building_time"].values
            t_querying = data[ (data["type"] == exec) & (data[iterable] == x) & (data["hits"] != -1)]["search_time"].values
            if len(t_building) == 0 or len(t_querying) == 0:
                X = X[0:len(mean_building)]
                break
            mean_building.append(statistics.mean(t_building)/1000)
            mean_search.append(statistics.mean(t_querying)/1000)
        ax_b.plot(X,mean_building, label=name[exec])
        ax_q.plot(X,mean_search, label=name[exec])
    ax_b.set(title='Building time', xlabel=iterable.title(), ylabel='Time (ms)', yscale='log')
    ax_q.set(title='Search time', xlabel=iterable.title(), ylabel='Time (ms)', yscale='log')
    ax_b.legend()
    ax_q.legend()
    
    X = data[data["type"] == "s"][iterable].unique()
    X.sort()
    mean_hits = []
    for x in X:
        t_hits = data[(data["type"] == "s")& (data[iterable] == x)]["hits"].values
        mean_hits.append(statistics.mean(t_hits))
    ax_h.plot(X,mean_hits)
    ax_h.set(title='Hits', xlabel=iterable.title(), ylabel='Hits', yticks=list(range(0,max(mean_hits)+1)))
    
    return fig
        
def plot2(data, iterable):
    heuristics = data["type"].unique()
    fig, axes = plt.subplots(math.ceil(len(heuristics)/2.0),2)
    axes = axes.flatten()
    for i in range(len(heuristics)):
        exec = heuristics[i]
        axis = axes[i]
        X = data[data["type"] == exec][iterable].unique()
        X.sort()
        mean_search = []
        for x in X:
            t_querying = data[ (data["type"] == exec) & (data[iterable] == x) & (data["hits"] != -1)]["search_time"].values
            if len(t_querying) == 0:
                X = X[0:len(mean_search)]
                break
            mean_search.append(statistics.mean(t_querying)/1000)
        
        if len(mean_search) < 1:
            axis.remove()
            continue
        
        axis.plot(X, mean_search, label="Original")
        
        res_lin = linregress(X,mean_search)
        axis.plot(X, res_lin.intercept + res_lin.slope*X, '--', label="O(n)")
        
        res_log = linregress(log(X),mean_search)
        axis.plot(X, res_log.intercept + res_log.slope*log(X), '--', label="O(log(n))")
        axis.set(title=name[exec], xlabel=iterable.title(), ylabel='Time (ms)', yscale='log')
        axis.legend()
        
        #print("*{}# *{:.5f}# *{:.5f}# {:.5f} & *{:.5f}# *{:.5f}# {:.5f}"
        #      .format(name[exec],res_lin.slope, res_lin.intercept,res_lin.rvalue**2,res_log.slope, res_log.intercept,res_log.rvalue**2))
        
        print("Linear regression. Heuristic: {}, R-squared: {}, Slope: {}, Bias: {}".format(exec,res_lin.rvalue**2,res_lin.slope, res_lin.intercept))
        print("Logarithm fitting. Heuristic: {}, R-squared: {}, Slope: {}, Bias: {}".format(exec,res_log.rvalue**2,res_log.slope, res_log.intercept))
        
        


data = pandas.read_csv('../results/neurons.csv')
fig1 = plot(data,"neurons")
fig1.suptitle('Timing for different neurons with density 16991 neurons/mm³')
#data = pandas.read_csv('neurons.csv')
plot2(data,"neurons")
print("Neurons plotted")

data = pandas.read_csv('../results/density.csv')
fig2 = plot(data, "density")
fig2.suptitle('Timing for different densities with 25230 neurons')
#data = pandas.read_csv('density.csv')
plot2(data,"density")
print("Density plotted")

data = pandas.read_csv('../results/density_2.csv')
fig3 = plot(data, "density")
fig3.suptitle('Timing for different densities with 25230 neurons')
#data = pandas.read_csv('density_2.csv')
plot2(data,"density")
print("Density_2 plotted")

plt.show()
