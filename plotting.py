import pandas
import statistics
import itertools
import matplotlib.pyplot as plt

data = pandas.read_csv('benchmark.csv')

columns = [ str(i) for i in range(1,1001)]
data["times"] = data[columns].values.tolist()
data.drop(labels=columns, axis=1, inplace=True)

data.plot(kind='hist')
plt.figure()
for exec in data["type"].unique():
    neurons = data[data["type"] == exec]["number_neurons"].unique()
    neurons.sort()
    means = []
    stdevs = []
    for number in neurons:
        times = list(itertools.chain.from_iterable(data[(data["type"] == exec) & (data["number_neurons"] == number)]["times"].values))
        means.append(statistics.mean(times))
        stdevs.append(statistics.stdev(times))
    plt.plot(neurons,means)
plt.legend(data["type"].unique())
plt.yscale('log')
plt.ylabel('Time (µs)')
plt.xlabel('Neurons')
plt.title('Average Building Time (N=1000)')
plt.show()