import React from "react";
import { Box, Button, Typography } from "@mui/material";
import { useAuth } from "../../context/AuthProvider";
import { useNavigate } from "react-router-dom";

const Home = () => {
	const { isAuthenticated } = useAuth();
	const navigate = useNavigate();

	return (
		<Box sx={{ padding: "3rem 0" }}>
			<Box
				sx={{
					display: "flex",
					flexDirection: "row",
					gap: "1rem",
					padding: { xs: "2rem 1.5rem 0", md: "2rem 3rem 0" },

					alignItems: "center",
					backgroundColor: "rgba(249, 250, 255, 1)",
				}}
			>
				<Box sx={{ textAlign: "left", flex: { xs: 1 } }}>
					<Typography
						sx={{
							fontSize: { xs: "1.875rem", sm: "3rem", md: "4rem" },
							fontWeight: 900,
							color: "rgba(18, 56, 144, 1)",
							lineHeight: { xs: "2rem", sm: "3rem", md: "4.5rem" },
						}}
					>
						Monitor Your Heart, Anywhere Anytime
					</Typography>
					<Button
						variant="contained"
						sx={{
							backgroundColor: "rgba(61, 119, 242, 1)",
							padding: { xs: "0.5rem 0.875rem", sm: "1rem 1.5rem" },
							marginTop: "1rem",
							borderRadius: "1rem",
							fontWeight: 600,
							fontSize: { xs: "0.875rem", sm: "1.25rem" },
						}}
						onClick={() => {
							if (isAuthenticated) {
								navigate("/analysis");
							} else {
								navigate("/login");
							}
						}}
					>
						Get Started
					</Button>
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
			<Box
				sx={{
					display: "flex",
					flexDirection: "row",
					gap: "1rem",
					justifyContent: "space-between",
					alignItems: "center",
					padding: { xs: "1rem 1.5rem", sm: "2rem 3rem", md: "2rem 5rem" },
				}}
			>
				<Box>
					<img src={"/images/piechart.png"} alt="Piechart" width={"70%"} />
					<Typography
						sx={{
							fontSize: { xs: "0.875rem", sm: "1.25rem" },
							lineHeight: { xs: "1rem", sm: "1.5rem" },
						}}
					>
						AI-Powered Analysis
					</Typography>
				</Box>
				<Box>
					<img
						src={"/images/heart-rate-pulse.png"}
						alt="Heart rate pulse"
						width={"70%"}
					/>
					<Typography
						sx={{
							fontSize: { xs: "0.875rem", sm: "1.25rem" },
							lineHeight: { xs: "1rem", sm: "1.5rem" },
						}}
					>
						Real Time Monitoring
					</Typography>
				</Box>
				<Box>
					<img src={"/images/download.png"} alt="Downlaod" width={"70%"} />
					<Typography
						sx={{
							fontSize: { xs: "0.875rem", sm: "1.25rem" },
							lineHeight: { xs: "1rem", sm: "1.5rem" },
						}}
					>
						Seamless Data Download
					</Typography>
				</Box>
			</Box>
		</Box>
	);
};

export default Home;
