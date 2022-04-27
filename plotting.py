import pandas
import statistics
import itertools
import matplotlib.pyplot as plt

data = pandas.read_csv('neurons.csv')
fig1, (ax_b, ax_q) = plt.subplots(1,2)
fig1.suptitle('Timing for different neurons with density 16991 neurons/mm³')
for exec in data["type"].unique():
    neurons = data[data["type"] == exec]["neurons"].unique()
    mean_building = []
    mean_search = []
    for neuron in neurons:
        t_building = data[ (data["type"] == exec) & (data["neurons"] == neuron)]["building_time"].values
        t_search = data[ (data["type"] == exec) & (data["neurons"] == neuron)]["search_time"].values
        mean_building.append(statistics.mean(t_building))
        mean_search.append(statistics.mean(t_search))
    ax_b.plot(neurons,mean_building, label=exec)
    ax_q.plot(neurons,mean_search, label=exec)
ax_b.set(title='Building time', xlabel='Neurons', ylabel='Time (µs)', yscale='log')
ax_q.set(title='Search time', xlabel='Neurons', ylabel='Time (µs)', yscale='log')
ax_b.legend()
ax_q.legend()

data = pandas.read_csv('density.csv')
fig2, (ax_b, ax_q) = plt.subplots(1,2)
fig2.suptitle('Timing for different densities with 25230 neurons')
for exec in data["type"].unique():
    densities = data[data["type"] == exec]["density"].unique()
    densities.sort()
    mean_building = []
    mean_search = []
    for density in densities:
        t_building = data[ (data["type"] == exec) & (data["density"] == density)]["building_time"].values
        t_search = data[ (data["type"] == exec) & (data["density"] == density)]["search_time"].values
        mean_building.append(statistics.mean(t_building))
        mean_search.append(statistics.mean(t_search))
    ax_b.plot(densities,mean_building, label=exec)
    ax_q.plot(densities,mean_search, label=exec)
ax_b.set(title='Building time', xlabel='Density', ylabel='Time (µs)', yscale='log')
ax_q.set(title='Search time', xlabel='Density', ylabel='Time (µs)', yscale='log')
ax_b.legend()
ax_q.legend()

plt.show()
