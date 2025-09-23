import { Close as CloseIcon } from "@mui/icons-material";
import { Box, Modal, Typography } from "@mui/material";
import { format } from "date-fns/format";
import React from "react";
import { downloadDeviceReadings } from "../../services/readings";

interface PrevReadingModalProps {
	open: boolean;
	data: any[];
	onClose?: () => void;
}
const PrevReadingModal = ({ open, onClose, data }: PrevReadingModalProps) => {

	const downloadReading = async (reading: any) => {
		const token = localStorage.getItem("cardiac_ai_access_token") || "";
		const timestamp = format(reading.timestamp, "dd-MM-yyyy:HH:mm:ss");

		const path = `CardiacAI_Readings_${timestamp}.png`;
		const response = await downloadDeviceReadings(reading.session_id, token);

		const link = document.createElement("a");
		link.href = URL.createObjectURL(response);
		link.download = path;
		link.click();
	};

	return (
		<Modal open={open} sx={{ padding: "2rem" }} onClose={onClose}>
			<Box
				sx={{
					width: { xs: "90%", md: "40%" },
					minHeight: "40vh",
					maxHeight: "60vh",
					overflowY: "auto",
					padding: { xs: "1rem", md: "2rem" },
					background: "#fff",
					borderRadius: "1rem",
					margin: "0 auto",
					marginTop: "5rem",
				}}
			>
				<CloseIcon onClick={onClose} sx={{ float: "right" }} />
				<Typography
					sx={{
						textAlign: "center",
						fontWeight: 600,
						fontSize: { xs: "1.25rem", md: "1.875rem" },
            borderBottom: "1px solid #000",
					}}
				>
					Previous Readings
				</Typography>
				<Box sx={{ overflow: "scroll", maxHeight: "50vh" }}>
					{data.map((reading, index) => (
						<Box
							key={index}
							sx={{
								padding: "1rem",
								border: "1px solid #ccc",
								borderRadius: "1rem",
								marginTop: "1rem",
								cursor: "pointer",
								":hover": {
									border: "2px solid rgba(47, 90, 196, 1)",
									background: "rgba(47, 90, 196, 1)",
									color: "#fff",
								},

								textAlign: "center",
							}}
							onClick={async () => {
								downloadReading(reading);
							}}
						>
							<Typography
								sx={{
									fontSize: { xs: "1rem", md: "1.25rem" },
								}}
							>
								Date:{" "}
								<span style={{ fontWeight: 600 }}>
									{format(new Date(reading?.timestamp), "EEEE, do MMMM yyyy")}
								</span>
							</Typography>
							<Typography
								sx={{
									fontSize: { xs: "1rem", md: "1.25rem" },
								}}
							>
								Time:{" "}
								<span style={{ fontWeight: 600 }}>
									{format(new Date(reading?.timestamp), "HH:mm:ss a")}
								</span>
							</Typography>
						</Box>
					))}
				</Box>
			</Box>
		</Modal>
	);
};

export default PrevReadingModal;
