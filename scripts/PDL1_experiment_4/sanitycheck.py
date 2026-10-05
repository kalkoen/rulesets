import pandas as pd
import seaborn as sns
from matplotlib import pyplot as plt
from sklearn.metrics import accuracy_score

X_TF_activities_broad = pd.read_csv("data/X_TFactivities_Broad.csv")
X = X_TF_activities_broad.pivot(values="TF activity score", index="sample", columns=["gene"])
y = pd.read_csv("data/Y_PDL1expression_Broad.csv", index_col="sample")["PDL1 expression"]
Xy = pd.concat([X, y], axis=1)

target = y >= y.quantile(0.5)
idx = (X["IRF1"] >= -1.589) & (X["STAT2"] >= -0.199)
hat = pd.Series(False, index=target.index)
hat[idx]=True
print(accuracy_score(target, hat))

target = y >= y.quantile(0.5)
idx = (X["STAT2"] >= -0.199)
hat = pd.Series(False, index=target.index)
hat[idx]=True
print(accuracy_score(target, hat))

print(Xy["PDL1 expression"].quantile(0.5))

Xy["PDL1_threshold"] = Xy["PDL1 expression"] >= Xy["PDL1 expression"].quantile(0.5)
ax = sns.scatterplot(data=Xy, x="STAT2", y="PDL1 expression", hue="PDL1_threshold", alpha=0.2)
ax.axvline(-0.199)
plt.show()


# ~ Finished.
# Final objective: 510.0
# Final accuracy: 0.6996466431095406
# Ruleset: [IRF1 >= -1.589 Q(0.100), STAT2 >= -0.199 Q(0.500)]


