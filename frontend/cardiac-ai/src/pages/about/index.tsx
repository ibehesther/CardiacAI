import { Box, Typography } from "@mui/material";
import React from "react";

const About = () => {
	return (
		<Box sx={{padding: '3rem 0'}}>
			<Box
				sx={{
					display: "flex",
					flexDirection: "row",
					gap: "1rem",
					padding: "2rem 2rem 0",
					alignItems: "center",
					backgroundColor: "rgba(249, 250, 255, 1)",
				}}
			>
				<Box sx={{ textAlign: "left", flex: { xs: 1 } }}>
					<Typography
						sx={{
							fontSize: { xs: "1.875rem", sm: "3rem", md: "3.5rem" },
							fontWeight: 900,
							color: "rgba(18, 56, 144, 1)",
							lineHeight: { xs: "2rem", sm: "3rem", md: "4rem" },
						}}
					>
						About the Device
					</Typography>
					<Typography
						sx={{
							fontSize: { xs: "1rem", sm: "1.25rem", md: "1.875rem" },
							lineHeight: { xs: "1.5rem", sm: "1.875rem", md: "2rem" },
						}}
					>
						A state-of-the-art device designed for ECG remote monitoring with
						AI-Powered interpretation. Enabling heart health assessment anytime,
						anywhere.
					</Typography>
				</Box>
				<Box sx={{ flex: { xs: 1, sm: 1 } }}>
					<img
						src="/images/health_monitoring.png"
						alt="health monitoring"
						width="100%"
					/>
				</Box>
			</Box>
			<Box
				sx={{
					width: "100%",
					height: { xs: "5rem", sm: "8rem" },
					borderRadius: "0 0 50% 50%",
					backgroundColor: "rgba(249, 250, 255, 1)",
				}}
			/>
			<Box sx={{ textAlign: "left", padding: "5rem 2rem" }}>
				<Typography
					sx={{
						fontSize: { xs: "1.875rem", sm: "3rem", md: "3.5rem" },
						fontWeight: 900,
						color: "rgba(18, 56, 144, 1)",
						lineHeight: { xs: "2rem", sm: "4rem" },
					}}
				>
					How It Works
				</Typography>
				<Typography
					sx={{
						maxWidth: { xs: "100%", sm: "60%" },
						fontSize: { xs: "1rem", sm: "1.25rem", md: "1.875rem" },
						lineHeight: { xs: "1.5rem", sm: "1.875rem", md: "2rem" },
					}}
				>
					The device records ECG data, then securely transmits it to a central
					system for analysis. Users can request AI-based interpretations or
					download ECG data directly.
				</Typography>
			</Box>
			<Box sx={{ textAlign: "left", padding: "2rem" }}>
				<Typography
					sx={{
						fontSize: { xs: "1.875rem", sm: "3rem", md: "3.5rem" },
						fontWeight: 900,
						color: "rgba(18, 56, 144, 1)",
						lineHeight: { xs: "2rem", sm: "3rem", md: "4rem" },
					}}
				>
					Project Owners{" "}
				</Typography>
				<Box
					sx={{
						width: "100%",
						display: "flex",
						flexDirection: "row",
						justifyContent: "space-between",
						alignItems: "center",
						gap: "1rem",
					}}
				>
					<Box sx={{ textAlign: "center" }}>
						<Box sx={{ borderRadius: "1rem" }}>
							<img
								src={"/images/heart-rate-pulse.png"}
								alt="Heart rate pulse"
								width={"70%"}
							/>
						</Box>
						<Typography
							sx={{
								textAlign: "center",
								fontSize: { xs: "0.875rem", sm: "1.25rem" },
								lineHeight: { xs: "1rem", sm: "1.5rem" },
							}}
						>
							Ibeh <br /> Esther Adaugo
						</Typography>
					</Box>
					<Box sx={{ textAlign: "center" }}>
						<Box sx={{ borderRadius: "1rem" }}>
							<img
								src={"/images/heart-rate-pulse.png"}
								alt="Heart rate pulse"
								width={"70%"}
							/>
						</Box>
						<Typography
							sx={{
								fontSize: { xs: "0.875rem", sm: "1.25rem" },
								lineHeight: { xs: "1rem", sm: "1.5rem" },
							}}
						>
							Adekoya <br />
							Adedeji Samson
						</Typography>
					</Box>
					<Box sx={{ textAlign: "center" }}>
						<Box sx={{ borderRadius: "1rem" }}>
							<img
								src={"/images/heart-rate-pulse.png"}
								alt="Heart rate pulse"
								width={"70%"}
							/>
						</Box>
						<Typography
							sx={{
								fontSize: { xs: "0.875rem", sm: "1.25rem" },
								lineHeight: { xs: "1rem", sm: "1.5rem" },
							}}
						>
							Arikewuyo <br />
							Muhammed-Buseri
						</Typography>
					</Box>
				</Box>
			</Box>
		</Box>
	);
};

export default About;
