import matplotlib.pyplot as plt
from io import BytesIO

"""
Utility functions for plotting ECG data and returning images.

All that is needed is just amplitude data and a session ID."""
def plot_ecg_and_return_image(session: dict):
    # We should look towards plotting only 1000 points
    ecg_data = session["data"]
    timestamp = session.get("timestamp", "")
    session_id = str(session.get("_id", ""))
    MAX_POINTS = 1000
    OFFSET = 50

    # X-axis: evenly spaced sample index (or timestamps if available)
    # x = list(range(len(ecg_data)))
    x = list(range(MAX_POINTS))
    if len(ecg_data) < MAX_POINTS:
        y = ecg_data
        x = x[:len(ecg_data)]
    elif len(ecg_data) < MAX_POINTS + OFFSET:
        y = ecg_data[:MAX_POINTS]
    else:
        y = ecg_data[OFFSET:MAX_POINTS+OFFSET]

    plt.figure(figsize=(12, 4))
    plt.plot(x, y, color="red")
    plt.title(f"ECG Session ({timestamp})")
    plt.xlabel(f"{len(x)} points random samples. SCALE: 100 = 1 second")
    plt.ylabel("Amplitude")
    plt.grid(True)

    buf = BytesIO()
    plt.savefig(buf, format="png")
    plt.close()
    buf.seek(0)
    return buf
