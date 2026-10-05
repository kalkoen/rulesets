import dataclasses
from typing import List

import matplotlib
from matplotlib.colors import LinearSegmentedColormap

cmap = LinearSegmentedColormap.from_list(
    "pastel_gr", ["#f5b8b8", "#b8e6b8"]  # faint pastel green -> faint pastel red
)

def to_hex(value, vmin, vmax):
    t = (value - vmin) / (vmax - vmin)
    r, g, b, _ = cmap(t)
    return f"{int(r*255):02X}{int(g*255):02X}{int(b*255):02X}"


@dataclasses.dataclass
class Node:
    children: List[Node]
    value: tuple
    score: float
    span: int = 0
    data: object = None


def leaf_count(node: Node):
    if not node.children:
        node.span = 1
        return 1
    node.span = sum(leaf_count(c) for c in node.children)
    return node.span


def levels(root):
    layer, out = [root], []
    while layer:
        out.append(layer)
        layer = [c for n in layer for c in n.children]
    return out


def to_latex(root: Node, vmin, vmax, depth_col=True):
    leaf_count(root)
    n = root.span
    rows = []
    for i, layer in enumerate(levels(root)):
        cells = " & ".join(f"\\multicolumn{{{n.span}}}{{|>{{\\columncolor[HTML]{{{to_hex(n.score, vmin, vmax)}}}}}c|}}{{{n.value[0]}}}" for n in layer)
        rows.append(((r"\textbf{Depth} & " if i == 0 else str(i) + " & ") if depth_col else '') + cells + r" \\")

        cells = " & ".join(f"\\multicolumn{{{n.span}}}{{|>{{\\columncolor[HTML]{{{to_hex(n.score, vmin, vmax)}}}}}c|}}{{{n.value[1]}}}" for n in layer)
        rows.append((" & " if depth_col else '') + cells + r" \\ \hline")
    body = "\n".join(rows)
    first_col = "| c" if depth_col else "|"
    return f"\\begin{{tabular}}{{{first_col}{'|c' * n}|}}\\hline\n{body}\n\\end{{tabular}}"

#
# print(to_latex(Node(
#     value="Root",
#     children=[
#         Node(value="C1", children=[
#             Node(value="C1.1", children=[],score=0.7),
#             Node(value="C1.2", children=[], score=0.6),
#         ], score= 1.0),
#         Node(value="C2", children=[
#             Node(value="C2.1", children=[],score=0.9),
#             Node(value="C2.2", children=[], score=0.8),
#         ], score= 0.65),
#         Node(value="C3", children=[
#             Node(value="C3.1", children=[],score=0.75),
#             Node(value="C3.2", children=[], score=0.8),
#         ], score= 1.0)
#     ],
#     score = 0.5)))
