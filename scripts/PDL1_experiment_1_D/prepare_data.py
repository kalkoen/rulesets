import os

Ds = range(1, 10)


folder_out = "data/PDL1_experiment_1_D/"
os.makedirs(folder_out, exist_ok=True)


file_format_out = os.path.join(folder_out, "settings_D_{d}.set")

settings = """brs/method = 1
brs/cg_timelimit = 7200
limits/time = 10800
brs/exact/D = {d}
brs/heuristic/beam_widths = "{beam_widths}"
"""

for d in Ds:
    beam_widths = ["500"]*d
    beam_widths[0] = "1000"
    out_string = settings.format(d=d, beam_widths=','.join(beam_widths))
    print(out_string)
    with open(file_format_out.format(d=d), "w") as f:
        f.write(out_string)