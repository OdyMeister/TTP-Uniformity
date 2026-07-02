import collections
import sys
import matplotlib.pyplot as plt
import scipy.stats as stats


SHOW_PLOT = False
SAVE_PLOT = True

MAX_SAMPLES = 10**5
FONT_SIZE = 32
FONT_SIZE_SMALL = 24

EXP = MAX_SAMPLES / 160
P = 0


def read_line_counts(path):
    with open(path, "r", encoding="utf-8") as file:
        i = 0
        schedules = []

        for schedule in file:
            if not schedule.startswith("2026") and i < MAX_SAMPLES:
                schedules.append(schedule.rstrip("\n"))
                i += 1
                
    schedules.sort()
    return collections.Counter(schedules)


def plot_counts(counts, ax, title, max_y=None):
    values = list(counts.values())
    positions = list(range(len(values)))

    global P
    P = stats.chisquare(values, f_exp=[EXP])[1]

    ax.bar(positions, values, color="#d66c36", edgecolor="black")

    step = max(1, len(positions) // 15)
    ax.set_xticks(positions[::step])
    ax.set_xticklabels([str(i) for i in positions[::step]], rotation=90, fontsize=FONT_SIZE_SMALL)

    ax.tick_params(axis="y", labelsize=FONT_SIZE_SMALL)

    bbox_props = dict(boxstyle="round,pad=0.4", facecolor="#E0E0E0", edgecolor="black", linewidth=0.8, alpha=0.9)
    ax.text(
        0.034,
        0.95,
        f"{title}\nPearson P={P:.3f}",
        transform=ax.transAxes,
        fontsize=FONT_SIZE_SMALL,
        verticalalignment="top",
        bbox=bbox_props,
    )

    if max_y is not None:
        ax.set_ylim(0, max_y * 1.05)

    ax.grid(axis="y", linestyle="--", alpha=0.6)


def max_min_info(counts):
    values = list(counts.values())
    if not values:
        return 0, None, 0, None
    max_val = max(values)
    min_val = min(values)
    return max_val, values.index(max_val), min_val, values.index(min_val)


def main():
    if len(sys.argv) != 3:
        print("Usage: python uniform_plot.py <file_RF.txt> <file_DFS.txt>")
        sys.exit(1)

    file2, file1 = sys.argv[1], sys.argv[2]
    counts_RF = read_line_counts(file1)
    counts_DFS = read_line_counts(file2)

    fig, axes = plt.subplots(1, 2, figsize=(18, 6), constrained_layout=True, sharey=True)
    # fig.suptitle("Random schedule sampling frequency distribution of 100k samples", fontsize=14)

    axes[0].set_ylabel("Frequency", fontsize=FONT_SIZE)

    max_count = max(
        max(counts_RF.values(), default=0),
        max(counts_DFS.values(), default=0),
    )

    plot_counts(counts_RF, axes[0], title="Row-first\n$11.19 \\times 10^{9}$ placements", max_y=max_count)

    max_count_RF, max_idx_RF, min_count_RF, min_idx_RF = max_min_info(counts_RF)

    print(f"Row-first sampling:")
    print(f"Max count for {file1}: {max_count_RF} at index {max_idx_RF}")
    print(f"Min count for {file1}: {min_count_RF} at index {min_idx_RF}")
    print("Chi-squared test:", P, "\n")

    plot_counts(counts_DFS, axes[1], title="Randomized BT\n$7.05 \\times 10^{6}$ placements", max_y=max_count)

    max_count_DFS, max_idx_DFS, min_count_DFS, min_idx_DFS = max_min_info(counts_DFS)

    print(f"DFS sampling:")
    print(f"Max count for {file2}: {max_count_DFS} at index {max_idx_DFS}")
    print(f"Min count for {file2}: {min_count_DFS} at index {min_idx_DFS}")
    print("Chi-squared test:", P)

    fig.text(0.5, -0.1, "Schedule index", ha="center", fontsize=FONT_SIZE)

    if SAVE_PLOT:
        plt.savefig("uniform_plot.png", dpi=300, bbox_inches="tight")

    if SHOW_PLOT:
        plt.show()


if __name__ == "__main__":
    main()
